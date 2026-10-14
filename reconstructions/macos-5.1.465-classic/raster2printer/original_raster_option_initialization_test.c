#define _POSIX_C_SOURCE 200809L
#include "original_raster_option_initialization.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef struct {
    char seq[64];unsigned len;
    int enabled;int ppd_present;
    unsigned choice_index;
    classic_oem_cups_option option;
    unsigned char seen_descriptor[OEM_DESCRIPTOR_SIZE];
} mock;
static void mark(mock *m,char ch){assert(m->len<sizeof m->seq-1);m->seq[m->len++]=ch;m->seq[m->len]=0;}
static int file_open(void*ctx,const char*path,int mode){(void)ctx;(void)path;(void)mode;assert(0&&"argc=6 must use stdin");return -1;}
static void sleep_call(void*ctx,unsigned seconds){(void)ctx;(void)seconds;assert(0);}
static void *raster_open(void*ctx,int fd,int mode){mock*m=ctx;mark(m,'R');assert(fd==0&&mode==0);return m;}
static void *ppd_open(void*ctx,const char*path){mock*m=ctx;mark(m,'P');assert(path&&strcmp(path,"classic.ppd")==0);return m->ppd_present?m:NULL;}
static int parse(void*ctx,const char *options,int n,classic_oem_cups_option**out){mock*m=ctx;mark(m,'A');assert(strcmp(options,"PrintThroughPRN=ON")==0&&n==0);m->option=(classic_oem_cups_option){"PrintThroughPRN","ON"};*out=&m->option;return 1;}
static void defaults(void*ctx,void*ppd){mock*m=ctx;mark(m,'D');assert(m==ppd);}
static int markone(void*ctx,void*ppd,const char*n,const char*v){mock*m=ctx;mark(m,'M');assert(m==ppd&&strcmp(n,"PrintThroughPRN")==0&&strcmp(v,"ON")==0);return 0;}
static int markall(void*ctx,void*ppd,int n,classic_oem_cups_option*opts){mock*m=ctx;mark(m,'L');assert(m==ppd&&n==1&&opts==&m->option);return -3;}
static const char *choice(void*ctx,void*ppd,const char *name){mock*m=ctx;mark(m,'Q');assert(m==ppd);static const char*keys[]={"Resolution","PrintQuality","DebugMode","PrintThroughPRN","OptimizationLevel","LabelCutter"};assert(m->choice_index<6&&strcmp(keys[m->choice_index],name)==0);m->choice_index++;return strcmp(name,"PrintThroughPRN")==0?"ON":NULL;}
static void freeopt(void*ctx,int n,classic_oem_cups_option *opt){mock*m=ctx;mark(m,'F');assert(n==1&&opt==&m->option);}
static void closeppd(void*ctx,void*ppd){mock*m=ctx;mark(m,'C');assert(ppd==m);}
static void debug(void*ctx){mark(ctx,'B');}
static void selected(void*ctx,int enabled){mock*m=ctx;mark(m,'T');m->enabled=enabled;}
static void start(void*ctx,const unsigned char *d,unsigned len){mock*m=ctx;mark(m,'S');assert(len==OEM_DESCRIPTOR_SIZE);memcpy(m->seen_descriptor,d,len);}
/* No print-quality selected in this test, so the real table is never called.
 * Actual name-index behavior is covered by the existing quality tests. */
int classic_raster_quality_selector(const char *name){(void)name;assert(0);return -1;}
static classic_oem_option_initialization_io setup(mock*m){
    classic_oem_preflight_io p={m,file_open,sleep_call,raster_open,ppd_open,parse,defaults,markone,markall};
    classic_oem_option_initialization_io io={p,choice,freeopt,closeppd,debug,selected,start};return io;
}
int main(void){
    assert(setenv("PPD","classic.ppd",1)==0);
    char *argv[]={"raster2printer","1","user","title","1","PrintThroughPRN=ON",NULL};
    unsigned char d[OEM_DESCRIPTOR_SIZE];classic_oem_preflight_state state={0};
    int rc=777;mock m={0};m.ppd_present=1;
    FILE *err=tmpfile();assert(err);int old=dup(fileno(stderr));assert(old>=0);assert(dup2(fileno(err),fileno(stderr))>=0);
    classic_oem_option_initialization_io io=setup(&m);
    assert(classic_original_raster_initialize_job(&io,6,argv,d,&state,&rc)==1);
    assert(rc==777 && strcmp(m.seq,"RPADMLQQQQQQFCBTS")==0);
    assert(m.choice_index==6 && m.enabled==0);
    assert(classic_original_raster_option_u32(d,OEM_OPTIMIZATION_OFFSET)==1);
    assert(classic_original_raster_option_u32(d,OEM_PRINT_THROUGH_PRN_OFFSET)==1);
    assert(memcmp(d,m.seen_descriptor,sizeof d)==0);
    void *handle=NULL;
    memcpy(&handle,d,sizeof handle);
    assert(handle==&m); /* manufacturer's raster handle set before StartDocument */
    /* Missing PPD must return before accessing choices or opening debug. */
    memset(&m,0,sizeof m);m.ppd_present=0;io=setup(&m);
    assert(classic_original_raster_initialize_job(&io,6,argv,d,&state,&rc)==0);
    assert(rc==-1 && strcmp(m.seq,"RP")==0);
    assert(fflush(stderr)==0);assert(dup2(old,fileno(stderr))>=0);close(old);fclose(err);
    puts("Original raster preflight / choice / document integration checks passed");return 0;
}
