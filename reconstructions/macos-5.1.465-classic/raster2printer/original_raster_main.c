/* Original raster2printer _main, macOS classic Leitz Icon 5.1.465
 * x86_64 Mach-O 0x100004460..0x100004565.
 * Entry sequence translated directly: open log, diagnostic, reset
 * termination flag, install SIGTERM/SIGPIPE handlers, reset job size,
 * execute raster filter, close log, return filter status.
 * The original does not check signal(), log or close return values.
 * No additional argument validation or cleanup is introduced.
 */
#include <signal.h>
#include <stdio.h>
#include "original_raster_private_helpers.h"

extern void OpenLogFile(void);
extern void CloseLogFile(void);
extern int raster2printer(int, char **);
/* job_size is declared in the shared OEM-native/private helper header. */

int classic_original_raster_main(int argc, char **argv) {
    OpenLogFile();
    fprintf(stderr,"\nraster2printer.c:41:main(): raster filter started");
    terminate_flag=0;
    (void)signal(SIGTERM,signal_handler);
    (void)signal(SIGPIPE,SIG_IGN);
    job_size=0;
    int result=raster2printer(argc,argv);
    CloseLogFile();
    return result;
}
/* Only enable the real OEM entry when all original dependencies are linked;
 * keeping the existing independently tested CUPS adapter untouched. */
#ifdef CLASSIC_OEM_FINAL_MAIN
int main(int argc,char **argv){return classic_original_raster_main(argc,argv);}
#endif
