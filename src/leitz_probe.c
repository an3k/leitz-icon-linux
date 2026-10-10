#define _XOPEN_SOURCE 700
#include "leitz_probe_io.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
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
    if (name[0]!='l' || name[1]!='p' || name[2]<'0' || name[2]>'9') return 0;
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

static int acquire_lock(const char *port) {
    const char *dir = "/run/leitz-icon";
    if (mkdir(dir,0700)<0 && errno!=EEXIST) return -1;
    struct stat st;
    if (lstat(dir,&st)<0 || !S_ISDIR(st.st_mode) || st.st_uid!=geteuid() ||
        (st.st_mode & 077)!=0) return -1;
    char path[PATH_MAX];
    int n = snprintf(path,sizeof path,"%s/%s.lock",dir,port);
    if (n<0 || (size_t)n>=sizeof path) return -1;
    int fd = open(path,O_CREAT|O_RDWR|O_CLOEXEC|O_NOFOLLOW,0600);
    if (fd<0) return -1;
    if (fstat(fd,&st)<0 || !S_ISREG(st.st_mode) || st.st_uid!=geteuid() ||
        flock(fd,LOCK_EX|LOCK_NB)<0) {
        close(fd);return -1;
    }
    return fd; /* Lock held across every device operation. */
}

static void usage(const char *program) {
    fprintf(stderr,"Usage: %s --idle-confirmed --device /dev/usb/lpN --command status|version|printed-lines --output /path/new.bin\n",program);
    fprintf(stderr,"Must be run by authorized operator with CUPS queue idle and device stopped; exactly one query, no retry.\n");
}

int main(int argc,char **argv) {
    const char *device=NULL,*command=NULL,*output=NULL,*port=NULL;
    int confirmed=0;
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"--idle-confirmed") && !confirmed) confirmed=1;
        else if (i+1<argc && !strcmp(argv[i],"--device") && !device) device=argv[++i];
        else if (i+1<argc && !strcmp(argv[i],"--command") && !command) command=argv[++i];
        else if (i+1<argc && !strcmp(argv[i],"--output") && !output) output=argv[++i];
        else {usage(argv[0]);return 2;}
    }
    if (!confirmed || !device || !command || !output || !is_valid_node(device,&port)) {
        usage(argv[0]);return 2;
    }
    uint8_t opcode;
    if (!strcmp(command,"status")) opcode='A';
    else if (!strcmp(command,"version")) opcode='V';
    else if (!strcmp(command,"printed-lines")) opcode='P';
    else {usage(argv[0]);return 2;}
    if (geteuid()!=0) {fprintf(stderr,"ERROR: Probe requires root inside the isolated CUPS container.\n");return 4;}
    int lockfd = acquire_lock(port);
    if (lockfd<0) {fprintf(stderr,"ERROR: Printer busy or shared lock unavailable; no query sent.\n");return 4;}
    if (verify_usb_identity(port)<0) {
        fprintf(stderr,"ERROR: Wrong or unidentifiable USB device; no query sent.\n");close(lockfd);return 4;
    }
    int outputfd = open(output,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if (outputfd<0) {perror("ERROR: Refusing to overwrite/create capture");close(lockfd);return 4;}
    int fd = open(device,O_RDWR|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK|O_NOCTTY);
    if (fd<0) {perror("ERROR: USB open");goto fail;}
    struct stat st;
    if (fstat(fd,&st)<0 || !S_ISCHR(st.st_mode)) {
        fprintf(stderr,"ERROR: USB path is not a character device.\n");close(fd);goto fail;
    }
    uint8_t response[LEITZ_PROBE_MAX_RESPONSE];
    size_t received=0;
    const int result=leitz_probe_exchange(fd,opcode,response,sizeof response,&received,
                                           opcode=='P'?7000:5000,250);
    if (close(fd)<0) {perror("ERROR: Closing device");goto fail;}
    if (result!=0) {
        fprintf(stderr,"ERROR: Status exchange failed (%d), %zu bytes. No retry.\n",result,received);
        goto fail;
    }
    size_t written=0;
    while (written<received) {
        ssize_t n=write(outputfd,response+written,received-written);
        if (n<0 && errno==EINTR) continue;
        if (n<=0) {perror("ERROR: Capture file write");goto fail;}
        written+=(size_t)n;
    }
    if (fsync(outputfd)<0 || close(outputfd)<0) {
        fprintf(stderr,"ERROR: Could not fully flush capture.\n");
        unlink(output);close(lockfd);return 4;
    }
    printf("CAPTURED=%zu bytes\nFILE=%s\nCOMMAND=ESC %c\nNO_PRINT_SENT=YES\n",received,output,opcode);
    close(lockfd);
    return 0;
fail:
    close(outputfd);
    unlink(output);
    close(lockfd);
    return 4;
}
