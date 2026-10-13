/* Mach-O command2leitz: recovered CUPS *entry path*, not a full C recovery.
 * On macOS, compile using the system CUPS SDK and platform notification API.
 * On Linux the opaque ABI declarations below permit an OFFLINE build only.
 * No claim of executable equivalence, macOS build verification, or hardware
 * verification is made. The original leaves its command file open at exit.
 */
#include "command2leitz_recovered.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifdef __APPLE__
#include <cups/cups.h>
#include <cups/file.h>
#include <cups/ppd.h>
#include <cups/backend.h>
#include <notify.h>
/* cupsBackChannelRead is referenced by the classic Mach-O executable, but
 * recent Apple SDK headers no longer declare this legacy CUPS function.
 * Preserve the original external ABI; do not substitute another API. */
ssize_t cupsBackChannelRead(char *, int, double);
#else
/* The ABI types are opaque except for the first 'marked' byte and 'choice'
 * array in ppd_choice_t. These offsets are observed in the original Mach-O.
 * The production macOS build always uses platform-provided headers. */
typedef struct _cups_file_s cups_file_t;
typedef struct ppd_file_s ppd_file_t;
typedef struct { char marked; char choice[41]; } ppd_choice_t;
typedef struct { char *name; char *value; } cups_option_t;
cups_file_t *cupsFileOpen(const char *,const char *);
cups_file_t *cupsFileStdin(void);
char *cupsFileGetConf(cups_file_t *,char *,size_t,char **,int *);
int cupsFilePrintf(cups_file_t *,const char *,...);
int cupsFileClose(cups_file_t *);
ssize_t cupsBackChannelRead(char *,int,double);
int cupsParseOptions(const char *,int,cups_option_t **);
void cupsFreeOptions(int,cups_option_t *);
ppd_file_t *ppdOpenFile(const char *);
void ppdClose(ppd_file_t *);
void ppdMarkDefaults(ppd_file_t *);
int cupsMarkOptions(ppd_file_t *,int,cups_option_t *);
ppd_choice_t *ppdFindMarkedChoice(ppd_file_t *,const char *);
#endif

static int stdout_char(void *unused,int ch) {
  (void)unused;
  return putchar(ch);
}
static int stdout_flush(void *unused) {
  (void)unused;
  return fflush(stdout);
}
static ssize_t backchannel(void *unused,char *buffer,int count,double timeout) {
  (void)unused;
  return cupsBackChannelRead(buffer,count,timeout);
}
static void stderr_message(void *unused,const char *text) {
  (void)unused;
  fprintf(stderr,"%s\n",text);
}
static int leitz_log(void *unused,const char *text) {
  (void)unused;
  cups_file_t *fp=cupsFileOpen("/private/var/log/leitz.log","w");
  if (!fp) return 0;
  cupsFilePrintf(fp,"%s",text);
  cupsFileClose(fp);
  return 1;
}
static void leitz_notify(void *unused,const char *domain) {
  (void)unused;
#ifdef __APPLE__
  notify_post(domain);
#else
  (void)domain;
#endif
}

int main(int argc,char **argv) {
  cups_file_t *input = argc == 7 ? cupsFileOpen(argv[6],"r") : cupsFileStdin();
  if (argc == 7 && !input) return 1;
  /* The original expects a PPD, and fails with -1 on missing PPD. */
  ppd_file_t *ppd=ppdOpenFile(getenv("PPD"));
  if (!ppd) return -1;
  cups_option_t *options=NULL;
  int n=cupsParseOptions(argv[5],0,&options);
  ppdMarkDefaults(ppd);
  cupsMarkOptions(ppd,n,options);
  ppd_choice_t *choice=ppdFindMarkedChoice(ppd,"ShouldSend_ESC_A");
  int should_send=choice && !strcasecmp(choice->choice,"YES");
  cupsFreeOptions(n,options);
  ppdClose(ppd);

  classic_command_io io={NULL,stdout_char,stdout_flush,backchannel,
                         stderr_message,leitz_log,leitz_notify};
  char command[1024];
  char *value=NULL;
  int lineno=0;
  while (cupsFileGetConf(input,command,sizeof command,&value,&lineno)) {
    if (classic_command_dispatch(&io,command,value,should_send))
      return 1;
  }
  return 0;
}
