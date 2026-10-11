#define _XOPEN_SOURCE 700
#include "leitz_probe_io.h"
#include "leitz_probe_format.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#error "This tool requires O_NOFOLLOW"
#endif

static int is_valid_node(const char *path, const char **port) {
    const char *prefix = "/dev/usb/";
    if (!path || strncmp(path,prefix,strlen(prefix)) != 0) return 0;
    const char *name = path + strlen(prefix);
    if (strlen(name)<3 || name[0]!='l' || name[1]!='p' || name[2]<'0' || name[2]>'9') return 0;
    for (const char *p=name+2; *p; ++p) if (*p<'0' || *p>'9') return 0;
    *port = name;
    return 1;
}

static int file_equals(const char *path, const char *value) {
    char buffer[40];
    FILE *f = fopen(path,"r");
    if (!f) return 0;
    const int ok = fgets(buffer,sizeof buffer,f)!=NULL;
    const int close_ok = fclose(f)==0;
    if (!ok || !close_ok) return 0;
    buffer[strcspn(buffer,"\r\n")] = 0;
    return strcasecmp(buffer,value)==0;
}

static int verify_usb_identity(const char *port) {
    char path[PATH_MAX],resolved[PATH_MAX],vid[PATH_MAX],pid[PATH_MAX];
    int k = snprintf(path,sizeof path,"/sys/class/usbmisc/%s/device",port);
    if (k<0 || (size_t)k>=sizeof path || !realpath(path,resolved)) return -1;
    char *last = strrchr(resolved,'/');
    if (!last) return -1;
    *last = 0;
    k = snprintf(vid,sizeof vid,"%s/idVendor",resolved);
    if (k<0 || (size_t)k>=sizeof vid) return -1;
    k = snprintf(pid,sizeof pid,"%s/idProduct",resolved);
    if (k<0 || (size_t)k>=sizeof pid) return -1;
    return file_equals(vid,"289e") && file_equals(pid,"1000") ? 0 : -1;
}

static void usage(const char *program) {
    fprintf(stderr,
      "Usage: %s --idle-confirmed --device /dev/usb/lpN --command status|version|printed-lines\n",
      program);
    fprintf(stderr,
      "Manual single query only; printer must be idle and mechanically stopped. No retry.\n");
}

int main(int argc, char **argv) {
    const char *device=NULL,*command=NULL,*port=NULL;
    int confirmed=0;
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"--idle-confirmed") && !confirmed) confirmed=1;
        else if (i+1<argc && !strcmp(argv[i],"--device") && !device) device=argv[++i];
        else if (i+1<argc && !strcmp(argv[i],"--command") && !command) command=argv[++i];
        else {usage(argv[0]);return 2;}
    }
    if (!confirmed || !device || !command || !is_valid_node(device,&port)) {
        usage(argv[0]);return 2;
    }
    uint8_t opcode;
    if (!strcmp(command,"status")) opcode='A';
    else if (!strcmp(command,"version")) opcode='V';
    else if (!strcmp(command,"printed-lines")) opcode='P';
    else {usage(argv[0]);return 2;}

    if (geteuid()!=0) {
        fprintf(stderr,"ERROR: Probe requires root inside the isolated container.\n");
        return 4;
    }
    if (verify_usb_identity(port)<0) {
        fprintf(stderr,"ERROR: Wrong or unidentifiable USB device; no query sent.\n");
        return 4;
    }
    /* The Linux usblp driver enforces a single open() of /dev/usb/lpN.
     * No filesystem lock or extra bind mount is needed for this access path.
     * This does NOT detect mechanical activity after the prior close(). */
    const int fd=open(device,O_RDWR|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK|O_NOCTTY);
    if (fd<0) {
        if(errno==EBUSY)
            fprintf(stderr,"ERROR: usblp device already open (EBUSY); no query sent.\n");
        else perror("ERROR: USB open");
        return 4;
    }
    struct stat st;
    if (fstat(fd,&st)<0 || !S_ISCHR(st.st_mode)) {
        fprintf(stderr,"ERROR: USB path is not a character device.\n");
        close(fd);return 4;
    }
    uint8_t response[LEITZ_PROBE_MAX_RESPONSE];
    size_t received=0;
    const int result=leitz_probe_exchange(fd,opcode,response,sizeof response,&received,
                                           opcode=='P'?7000:5000,1000);
    const int close_ok=close(fd);
    if(result!=0 || close_ok<0) {
        if (result==LEITZ_PROBE_TIMEOUT && received==0) {
            printf("COMMAND=ESC %c\nREPLY_BYTES=0\nDECODED=NO_RESPONSE_WITHIN_TIMEOUT\n",opcode);
            fflush(stdout);
        }
        fprintf(stderr,"ERROR: Status exchange failed (%d), received %zu bytes; no retry.\n",
                result,received);
        return 4;
    }
    /* Format the recorded bytes after closing USB. Decoding is diagnostic
     * only: unknown values must not influence printing or recovery. */
    if (leitz_probe_format(stdout,opcode,response,received)) {
        fprintf(stderr,"ERROR: Cannot format USB response.\n");
        return 4;
    }
    return 0;
}
