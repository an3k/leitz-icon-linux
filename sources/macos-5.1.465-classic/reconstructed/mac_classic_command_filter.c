/* Classic macOS command2leitz CUPS entry, independently reconstructed.
 * OEM Mach-O _main: 0x100000b10..0x1000011c0
 * This adapter uses the CUPS 2.x public ABI to read the command file.
 * It does not implement proprietary macOS notification side effects.
 * For offline builds without SDK, the isolated ABI declarations are used;
 * they don't authorize assuming any other API layout is identical.
 */
#include "mac_classic_command_job.h"
#include "mac_classic_output.h"
#ifdef MAC_CLASSIC_USE_SYSTEM_CUPS_HEADERS
#include <cups/cups.h>
#include <cups/file.h>
#include <cups/backend.h>
#else
#include "mac_classic_command_abi.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <notify.h>
#endif

static ssize_t backchannel(void *ctx,uint8_t *buffer,size_t n,double timeout){
  (void)ctx;
  if(n>1024u)return -1;
  return cupsBackChannelRead((char *)buffer,(int)n,timeout);
}
/* This option-only parser is an intentional portability deviation:
 * the original uses ppdMarkDefaults + cupsMarkOptions +
 * ppdFindMarkedChoice("ShouldSend_ESC_A"); original _EndOfJob ignores
 * the flag. We retain the *observable* unconditional ESC Z behavior.
 */
static int optional_mac_archive(void *unused,const char *command,
                                const char *value,const uint8_t *data,size_t n){
  (void)unused;(void)command;
  const char *path=getenv("LEITZ_MAC_CLASSIC_STATUS_LOG");
  if(!path || !*path)return 0;
  /* Opt-in only: original overwrites /private/var/log/leitz.log and
   * notifies "com.xyron.Leitz-Icon-Software". Never overwrite files
   * implicitly during reverse engineering. */
  FILE *f=fopen(path,"wb");
  if(!f)return -1;
  size_t length=value?strlen(value):0;
  int ok=(!length||fwrite(value,1,length,f)==length) &&
         (!n||fwrite(data,1,n,f)==n);
  if(fclose(f))ok=0;
#ifdef __APPLE__
  if(ok && notify_post("com.xyron.Leitz-Icon-Software")!=0)ok=0;
#endif
  return ok?0:-1;
}
int main(int argc,char **argv){
  if(argc!=6 && argc!=7){
    fputs("ERROR: CUPS command filter expects 5 or 6 arguments\n",stderr);
    return 1;
  }
  cups_file_t *input=argc==7 ? cupsFileOpen(argv[6],"r") : cupsFileStdin();
  if(!input){fputs("ERROR: Unable to open command file\n",stderr);return 1;}
  int outfd=1;
  mac_classic_output output={MAC_CLASSIC_STDOUT,mac_classic_command_write_fd,
                             &outfd,NULL,NULL};
  mac_classic_status_io exchange={&output,backchannel,NULL};
  mac_classic_command_context ctx={&exchange,mac_classic_command_emit_stderr,
                                   stderr,optional_mac_archive,NULL,7.0};
  char command[1024];
  char *value=NULL;
  int lineno=0;
  unsigned processed=0,failures=0;
  /* Original also reads one CUPS config line per iteration. */
  while(cupsFileGetConf(input,command,sizeof command,&value,&lineno)){
    int result=mac_classic_command_run(&ctx,command,value);
    if(result<0)failures++;
    else processed++;
  }
  if(argc==7 && cupsFileClose(input)<0)failures++;
  if(!processed){fputs("ERROR: No recognized CUPS command completed\n",stderr);return 1;}
  return failures?1:0;
}
