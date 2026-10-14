/* Isolated CUPS-linked candidate using the reconstructed OEM _main branch
 * sequence. Does not replace existing command2leitz_cups.c or the OEM binary.
 * Deliberately keeps the original argc and PPD error semantics. */
#include "original_command_main_flow.h"
#include "command2leitz_recovered.h"
#include "original_command_file_helpers.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <cups/cups.h>
#include <cups/file.h>
#include <cups/ppd.h>
#include <cups/backend.h>
#include <notify.h>
ssize_t cupsBackChannelRead(char *, int, double);
#else
/* Offline build uses the same libcups ABI declaration pattern already
 * present in command2leitz_cups.c; not a production SDK replacement. */
typedef struct _cups_file_s cups_file_t;
typedef struct ppd_file_s ppd_file_t;
typedef struct {char marked;char choice[41];} ppd_choice_t;
typedef struct {char *name;char *value;} cups_option_t;
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
static void* open_file(void *ctx,const char*path,const char*mode){(void)ctx;return cupsFileOpen(path,mode);}
static void* std_file(void *ctx){(void)ctx;return cupsFileStdin();}
static void* ppd_open(void *ctx,const char*path){(void)ctx;return ppdOpenFile(path);}
static int parse_opts(void *ctx,const char*str,void **opts){(void)ctx;return cupsParseOptions(str,0,(cups_option_t**)opts);}
static void mark_defaults(void *ctx,void *ppd){(void)ctx;ppdMarkDefaults((ppd_file_t*)ppd);}
static void mark_opts(void *ctx,void *ppd,int n,void *opts){(void)ctx; cupsMarkOptions((ppd_file_t*)ppd,n,(cups_option_t*)opts);}
static const char *choice(void *ctx,void *ppd,const char *key){(void)ctx;ppd_choice_t *c=ppdFindMarkedChoice((ppd_file_t*)ppd,key);return c?c->choice:NULL;}
static void free_opts(void *ctx,int n,void *opts){(void)ctx;cupsFreeOptions(n,(cups_option_t*)opts);}
static void close_ppd(void *ctx,void*ppd){(void)ctx;ppdClose((ppd_file_t*)ppd);}
static const char *get_conf(void *ctx,void*file,char *name,size_t sz,char **value,int *ln){(void)ctx;return cupsFileGetConf((cups_file_t*)file,name,sz,value,ln);}
static int out_char(void *ctx,int ch){(void)ctx;return putchar(ch);}
static int out_flush(void *ctx){(void)ctx;FlushDataStreams();return 0;}
static ssize_t back(void *ctx,char *buf,int max,double seconds){(void)ctx;return cupsBackChannelRead(buf,max,seconds);}
static void msg(void *ctx,const char*s){
  (void)ctx;
  /* OEM _GetStatus emits STATE lines with a preceding newline and no
   * trailing newline; the NoData / ATTR lines have the conventional
   * CUPS terminal newline. Compare against verified OEM stdout/stderr. */
  if(!strncmp(s,"STATE: -",8))
    fprintf(stderr,"\n%s\n",s);
  else if(!strcmp(s,"STATE: +media-jam-error,com.leitz.paper-jam-error") ||
          !strcmp(s,"STATE: +com.leitz.out-of-paper-warning,media-empty-warning") ||
          !strcmp(s,"STATE: +com.leitz.general-error"))
    fprintf(stderr,"\n%s\n",s);
  else if(!strncmp(s,"STATE:",6))
    fprintf(stderr,"\n%s",s);
  else fprintf(stderr,"%s\n",s);
}
static int log_file(void *ctx,const char*text){
  (void)ctx;
  cups_file_t *f=cupsFileOpen("/private/var/log/leitz.log","w");
  if(!f) {
    /* OEM _GetStatus at original source line 208 prints strerror(errno)
     * followed by one literal space, without a final newline. */
    int error=errno;
    fprintf(stderr,
      "\ncommand2leitz.c:208:GetStatus(): Failed while opening leitz.log file, error = %s \n",
      strerror(error));
  } else {
    cupsFilePrintf(f,"%s",text);
    cupsFileClose(f);
  }
  fprintf(stderr,
    "\ncommand2leitz.c:217:GetStatus(): data for state-message = : %s\n",
    text);
  return f!=NULL;
}
static void notify(void *ctx,const char*domain){(void)ctx;
#ifdef __APPLE__
  notify_post(domain);
#else
  (void)domain;
#endif
}
static int query(void *ctx,char command,const char*prefix,int send,double timeout){
  (void)ctx;
  classic_command_io io={NULL,out_char,out_flush,back,msg,log_file,notify};
  return classic_command_get_status(&io,command,prefix,send,timeout);
}
static int query_a(void*ctx,int flag,double timeout){return query(ctx,'A',"PRINTER_STATUS_",flag,timeout);}
static int query_u(void*ctx,int flag,double timeout){return query(ctx,'U',"UNITED_STATUS_",flag,timeout);}
static int query_v(void*ctx,int flag,double timeout){return query(ctx,'V',"FIRMWARE_VERSION_",flag,timeout);}
int main(int argc,char **argv){
  classic_command_main_ops o={NULL,open_file,std_file,ppd_open,parse_opts,
    mark_defaults,mark_opts,choice,free_opts,close_ppd,get_conf,query_a,query_u,query_v};
  return classic_original_command_main(&o,argc,argv,getenv("PPD"));
}
