#define _POSIX_C_SOURCE 200809L
#include "original_raster_preflight.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char calls[64];int n;
    int file_fd;
    int raster_present,ppd_present;
    int conflicts;
    unsigned sleeps;
    classic_oem_cups_option choices[3];
} mock;
static void event(mock*m,char c){assert(m->n<(int)sizeof m->calls-1);m->calls[m->n++]=c;m->calls[m->n]=0;}
static int open_input(void *p,const char *file,int flags){mock*m=p;event(m,'O');assert(strcmp(file,"raster.bin")==0&&flags==0);return m->file_fd;}
static void sleep_seconds(void*p,unsigned n){mock*m=p;event(m,'S');assert(n==1);m->sleeps++;}
static void *raster_open(void*p,int fd,int mode){mock*m=p;event(m,'R');assert(mode==0);assert(fd==0||fd==9);return m->raster_present?m:NULL;}
static void *ppd_open(void*p,const char*path){mock*m=p;event(m,'P');assert(path&&strcmp(path,"classic.ppd")==0);return m->ppd_present?m:NULL;}
static int parse_opts(void*p,const char *str,int n,classic_oem_cups_option **opts){mock*m=p;event(m,'A');assert(n==0 && strcmp(str,"Resolution=600dpi foo=bad NullOption")==0);*opts=m->choices;return 3;}
static void mark_defaults(void*p,void*ppd){mock*m=p;event(m,'D');assert(ppd==m);}
static int mark_option(void*p,void*ppd,const char*name,const char*value){mock*m=p;event(m,'M');assert(ppd==m);assert(name&&value);return strcmp(name,"foo")==0?m->conflicts:0;}
static int mark_all(void*p,void*ppd,int n,classic_oem_cups_option*o){mock*m=p;event(m,'L');assert(ppd==m&&n==3&&o==m->choices);return -99;}
static classic_oem_preflight_io callbacks(mock*m){classic_oem_preflight_io io={m,open_input,sleep_seconds,raster_open,ppd_open,parse_opts,mark_defaults,mark_option,mark_all};return io;}
static void prepare(mock *m){memset(m,0,sizeof *m);m->file_fd=9;m->raster_present=1;m->ppd_present=1;m->conflicts=2;m->choices[0]=(classic_oem_cups_option){"Resolution","600dpi"};m->choices[1]=(classic_oem_cups_option){"foo","bad"};m->choices[2]=(classic_oem_cups_option){"NullOption",NULL};}
static int captured(mock*m,int argc,char**argv,classic_oem_preflight_state *state,int*early,char *out,size_t sz){
    FILE *f=tmpfile();assert(f);int save=dup(fileno(stderr));assert(save>=0);assert(dup2(fileno(f),fileno(stderr))>=0);
    classic_oem_preflight_io io=callbacks(m);
    int n=classic_original_raster_preflight(&io,argc,argv,state,early);
    assert(fflush(stderr)==0);assert(dup2(save,fileno(stderr))>=0);assert(close(save)==0);
    rewind(f);size_t bytes=fread(out,1,sz-1,f);out[bytes]=0;assert(fclose(f)==0);return n;
}
int main(void){
  assert(setenv("PPD","classic.ppd",1)==0);
  char *argv[]={"raster2printer","job","user","name","1","Resolution=600dpi foo=bad NullOption","raster.bin",NULL};
  classic_oem_preflight_state state={0};mock m;int early=123;char out[2048];
  prepare(&m);assert(captured(&m,5,argv,&state,&early,out,sizeof out)==0);
  assert(early==1 && m.n==0);
  assert(strcmp(out,"\nraster2printer.c:76:raster2printer(): error: (argc < 6) || (argc > 7)")==0);
  prepare(&m);m.file_fd=-1;assert(captured(&m,7,argv,&state,&early,out,sizeof out)==0);
  assert(early==1 && m.sleeps==1 && strcmp(m.calls,"OS")==0);
  assert(strcmp(out,"\nraster2printer.c:83:raster2printer(): opening file from argv[6]='raster.bin'\nraster2printer.c:86:raster2printer(): cannot open file argv[6]='raster.bin'")==0);
  prepare(&m);m.raster_present=0;errno=ENOENT;
  assert(captured(&m,6,argv,&state,&early,out,sizeof out)==0);
  assert(early==1 && strcmp(m.calls,"R")==0);
  assert(strstr(out,"ERROR: Unable to open raster file - : No such file or directory")!=NULL);
  prepare(&m);m.ppd_present=0;errno=ENOENT;
  assert(captured(&m,6,argv,&state,&early,out,sizeof out)==0);
  assert(early==-1 && strcmp(m.calls,"RP")==0);
  assert(strstr(out,"WARNING: Unable to open ppd file, use default settings - : No such file or directory")!=NULL);
  prepare(&m);early=123;
  assert(captured(&m,7,argv,&state,&early,out,sizeof out)==1);
  assert(early==123 && strcmp(m.calls,"ORPADMML")==0);
  assert(state.input_fd==9 && state.ppd==&m && state.raster==&m && state.option_count==3 && state.options==m.choices);
  const char *prefix=
  "\nraster2printer.c:83:raster2printer(): opening file from argv[6]='raster.bin'"
  "\nraster2printer.c:115:raster2printer(): option[0]:'Resolution'='600dpi'"
  "\nraster2printer.c:115:raster2printer(): option[1]:'foo'='bad'"
  "\nraster2printer.c:123:raster2printer(): got 2 conflicts for option 'foo'"
  "\nraster2printer.c:115:raster2printer(): option[2]:'NullOption'='";
  assert(strncmp(out,prefix,strlen(prefix))==0);
  assert(strstr(out,"\nraster2printer.c:128:raster2printer(): process options")!=NULL);
  puts("Original classic CUPS raster preflight branches and output verified");
  return 0;
}
