#include "original_command_main_flow.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

/* Original command2leitz Mach-O 0x100000b10..0x1000011c0.
 * Every diagnostic below uses the manufacturer's original file basename,
 * original line number and original message, not our source __LINE__.
 * The original argc != 7 branch takes cupsFileStdin() unconditionally;
 * even argc < 6 is not rejected before argv[5] is accessed.
 * Do not add cleanup calls: the original _main does not close its input file.
 */
#define OEM_LINE(number, message) \
    fprintf(stderr, "\ncommand2leitz.c:%d:main(): %s",number,message)

int classic_original_command_main(const classic_command_main_ops *ops,
                                  int argc, char **argv, const char *ppd_path) {
    void *input;
    void *ppd;
    void *options = NULL;
    int n;
    int should_send_esc_a=0;
    char name[1024];
    char *value=NULL;
    int line=0;

    OEM_LINE(41,"command filter started\n");
    if(argc==7) {
        input=ops->file_open(ops->context,argv[6],"r");
        if (!input) {
            OEM_LINE(49,"ERROR while opening command file...\n");
            return 1;
        }
        OEM_LINE(54,"command file opened successfully...\n");
    } else {
        input=ops->file_stdin(ops->context);
    }
    ppd=ops->ppd_open(ops->context,ppd_path);
    if(!ppd) {
        /* The OEM performs perror and returns -1; it does not continue
         * with its otherwise mentioned 'use default settings'. */
        perror("WARNING: Unable to open ppd file, use default settings - ");
        return -1;
    }
    n=ops->parse_options(ops->context,argv[5],&options);
    ops->mark_defaults(ops->context,ppd);
    OEM_LINE(75,"process options\n");
    ops->mark_options(ops->context,ppd,n,options);
    const char *choice=ops->marked_choice(ops->context,ppd,"ShouldSend_ESC_A");
    if(choice) {
        should_send_esc_a=(strcasecmp(choice,"YES")==0);
        fprintf(stderr,"\ncommand2leitz.c:85:main(): should send ESC, A : %s\n",choice);
    }
    ops->free_options(ops->context,n,options);
    ops->ppd_close(ops->context,ppd);

    while (ops->file_get_conf(ops->context,input,name,sizeof(name),&value,&line)) {
        OEM_LINE(98,"Parse the command...\n");
        if(strcasecmp(name,"ReportStatus")==0) {
            ops->get_printer_status(ops->context,should_send_esc_a,7.0);
            return 1;   /* OEM ignores status return value and exits 1 */
        }
        if(strcasecmp(name,"GetUnitedStatus")==0) {
            ops->get_united_status(ops->context,should_send_esc_a,7.0);
        } else if(strcasecmp(name,"GetFirmwareVersion")==0) {
            ops->get_firmware_version(ops->context,should_send_esc_a,7.0);
        }
    }
    return 0;
}
