#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int classic_original_raster_main(int,char**);
int terminate_flag=13;unsigned int job_size=22;
static char calls[5];static int n;
static int filter_status;
void OpenLogFile(void){calls[n++]='O';}
void CloseLogFile(void){calls[n++]='C';}
void signal_handler(int signo){(void)signo;}
int raster2printer(int argc,char **argv){
    calls[n++]='R';
    assert(argc==6 && strcmp(argv[0],"raster2printer")==0);
    assert(terminate_flag==0 && job_size==0);
    struct sigaction term,pipe;
    assert(sigaction(SIGTERM,NULL,&term)==0);
    assert(sigaction(SIGPIPE,NULL,&pipe)==0);
    assert(term.sa_handler==signal_handler && pipe.sa_handler==SIG_IGN);
    return filter_status;
}
int main(void){
    struct sigaction oldterm,oldpipe;
    assert(sigaction(SIGTERM,NULL,&oldterm)==0);
    assert(sigaction(SIGPIPE,NULL,&oldpipe)==0);
    FILE *f=tmpfile();assert(f);
    int orig=dup(fileno(stderr));assert(orig>=0);
    assert(dup2(fileno(f),fileno(stderr))>=0);
    char *args[]={"raster2printer","201","user","title","1","Resolution=300dpi",NULL};
    filter_status=7;
    assert(classic_original_raster_main(6,args)==7);
    assert(n==3 && !memcmp(calls,"ORC",3));
    filter_status=-1;n=0;terminate_flag=8;job_size=999;
    assert(classic_original_raster_main(6,args)==-1);
    assert(n==3 && !memcmp(calls,"ORC",3));
    assert(fflush(stderr)==0);
    assert(dup2(orig,fileno(stderr))>=0);assert(close(orig)==0);
    assert(sigaction(SIGTERM,&oldterm,NULL)==0);
    assert(sigaction(SIGPIPE,&oldpipe,NULL)==0);
    rewind(f);char diag[256]={0};size_t k=fread(diag,1,255,f);diag[k]=0;assert(fclose(f)==0);
    assert(strcmp(diag,
      "\nraster2printer.c:41:main(): raster filter started"
      "\nraster2printer.c:41:main(): raster filter started")==0);
    puts("Classic OEM raster main call order, signals and exact diagnostics passed");
    return 0;
}
