#define _POSIX_C_SOURCE 200809L
#include "original_command_main_flow.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char trace[256];size_t t;
    const char *names[5];unsigned index;
    const char *choice;
    int file_available,ppd_available;
    int status_return;
    unsigned saw_argv_file,saw_stdin,nparsed,last_options;
    unsigned got_status,got_united,got_firmware;
    int flags[4];double timeouts[4];
} mock;
static void add(mock *m,char event){assert(m->t+1<sizeof m->trace);m->trace[m->t++]=event;m->trace[m->t]=0;}
static void *open_file(void *p,const char *name,const char *mode){mock *m=p;add(m,'O');assert(strcmp(name,"control.txt")==0&&strcmp(mode,"r")==0);m->saw_argv_file++;return m->file_available?m:NULL;}
static void *stdin_file(void *p){mock *m=p;add(m,'I');m->saw_stdin++;return m;}
static void *ppd_open(void *p,const char *name){mock *m=p;add(m,'P');assert(strcmp(name,"vendor.ppd")==0);return m->ppd_available?m:NULL;}
static int parse(void *p,const char *value,void **result){mock*m=p;add(m,'R');assert(strcmp(value,"OptimizationLevel=2")==0);*result=m;m->nparsed++;return 2;}
static void defaults(void *p,void *ppd){mock*m=p;assert(ppd==m);add(m,'D');}
static void mark(void *p,void *ppd,int n,void*o){mock*m=p;assert(ppd==m&&o==m&&n==2);add(m,'M');m->last_options=(unsigned)n;}
static const char *choice(void *p,void *ppd,const char*name){mock*m=p;assert(ppd==m&&strcmp(name,"ShouldSend_ESC_A")==0);add(m,'C');return m->choice;}
static void freeopts(void *p,int n,void*o){mock*m=p;assert(n==2&&o==m);add(m,'F');}
static void close_ppd(void *p,void*ppd){mock*m=p;assert(ppd==m);add(m,'X');}
static const char *conf(void *p,void*input,char*dst,size_t max,char**value,int*line){mock*m=p;assert(input==m&&max==1024);add(m,'G');const char*name=m->names[m->index++];if(!name)return NULL;strcpy(dst,name);*value=(char*)"YES";(*line)++;return dst;}
static int status(void *p,int flag,double timeout){mock*m=p;add(m,'S');m->got_status++;m->flags[0]=flag;m->timeouts[0]=timeout;return m->status_return;}
static int united(void *p,int flag,double timeout){mock*m=p;add(m,'U');m->got_united++;m->flags[1]=flag;m->timeouts[1]=timeout;return m->status_return;}
static int firmware(void *p,int flag,double timeout){mock*m=p;add(m,'V');m->got_firmware++;m->flags[2]=flag;m->timeouts[2]=timeout;return m->status_return;}
static void init(mock*m){memset(m,0,sizeof* m);m->file_available=1;m->ppd_available=1;m->choice="YES";}
static classic_command_main_ops ops(mock*m){classic_command_main_ops x={m,open_file,stdin_file,ppd_open,parse,defaults,mark,choice,freeopts,close_ppd,conf,status,united,firmware};return x;}
static int run(mock*m,int argc,char**argv,char *log,size_t max){
    FILE *f=tmpfile();assert(f);int fd=dup(fileno(stderr));assert(fd>=0);assert(dup2(fileno(f),fileno(stderr))>=0);
    classic_command_main_ops o=ops(m);int ret=classic_original_command_main(&o,argc,argv,"vendor.ppd");
    assert(fflush(stderr)==0);assert(dup2(fd,fileno(stderr))>=0);assert(close(fd)==0);rewind(f);size_t n=fread(log,1,max-1,f);log[n]=0;assert(fclose(f)==0);return ret;
}
int main(void){
    char *argv[]={"command2leitz","201","owner","name","1","OptimizationLevel=2","control.txt",NULL};
    char log[4096];mock m;init(&m);
    m.names[0]="GetUnitedStatus";m.names[1]="GETFIRMWAREVERSION";m.names[2]="unknown";
    assert(run(&m,7,argv,log,sizeof log)==0);
    assert(strcmp(m.trace,"OPRDMCFXGUGVGG")==0);
    assert(m.got_united==1&&m.got_firmware==1&&m.got_status==0);
    assert(m.flags[1]==1&&m.flags[2]==1&&m.timeouts[1]==7.0&&m.timeouts[2]==7.0);
    assert(strcmp(log,
      "\ncommand2leitz.c:41:main(): command filter started\n"
      "\ncommand2leitz.c:54:main(): command file opened successfully...\n"
      "\ncommand2leitz.c:75:main(): process options\n"
      "\ncommand2leitz.c:85:main(): should send ESC, A : YES\n"
      "\ncommand2leitz.c:98:main(): Parse the command...\n"
      "\ncommand2leitz.c:98:main(): Parse the command...\n"
      "\ncommand2leitz.c:98:main(): Parse the command...\n")==0);
    init(&m);m.names[0]="reportstatus";m.names[1]="GetFirmwareVersion";m.status_return=0;
    assert(run(&m,7,argv,log,sizeof log)==1);
    assert(m.got_status==1&&m.got_firmware==0&&m.index==1);
    init(&m);m.file_available=0;assert(run(&m,7,argv,log,sizeof log)==1);
    assert(strcmp(m.trace,"O")==0&&strstr(log,"command2leitz.c:49:main(): ERROR while opening command file...\n"));
    init(&m);m.ppd_available=0;errno=ENOENT;assert(run(&m,6,argv,log,sizeof log)==-1);
    assert(strcmp(m.trace,"IP")==0&&strstr(log,"WARNING: Unable to open ppd file, use default settings - "));
    init(&m);m.choice=NULL;m.names[0]=NULL;
    assert(run(&m,6,argv,log,sizeof log)==0);assert(m.saw_stdin==1&&m.saw_argv_file==0);
    assert(!strstr(log,"should send ESC, A :"));
    init(&m);m.choice="no";m.names[0]="GetUnitedStatus";
    assert(run(&m,6,argv,log,sizeof log)==0);assert(m.flags[1]==0);
    puts("classic OEM command _main branching, ordering and diagnostics checks passed");return 0;
}
