#include "monitor2_open_port.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {char log[32];unsigned n;int exists,fail_allocate,fail_open,freed,errors;uint32_t last_error;void *monitor,*handle;leitz_lm_open_port *last;} trace;
static void step(trace *t,char x){assert(t->n<sizeof t->log-1);t->log[t->n++]=x;t->log[t->n]=0;}
static void lock(void *v){step(v,'L');}
static void unlock(void *v){step(v,'U');}
static int32_t exists(void *v,const uint16_t *n){trace*t=v;assert(n[0]=='U');step(t,'D');return t->exists;}
static leitz_lm_open_port *alloc(void *v,const uint16_t *n){trace*t=v;assert(n[1]=='S');step(t,'A');return t->fail_allocate?NULL:calloc(1,sizeof(leitz_lm_open_port));}
static void free_port(void *v,leitz_lm_open_port *p){trace*t=v;step(t,'F');++t->freed;free(p);}
static int32_t host(void *v,void *m,const uint16_t *n,void **h){trace*t=v;assert(m==t->monitor&&n[2]=='B');step(t,'H');if(t->fail_open)return 0;*h=t->handle;return 1;}
static void err(void *v,uint32_t c){trace*t=v;step(t,'E');t->errors++;t->last_error=c;}
static void record(void *v,leitz_lm_open_port *p){trace*t=v;step(t,'R');t->last=p;}
int main(void){
    static const uint16_t usb[]={'U','S','B',0};
    int monitor=42,handle=77;
    trace t={.monitor=&monitor,.handle=&handle};
    const leitz_lm_open_callbacks c={&t,lock,unlock,exists,alloc,free_port,host,err,record};
    uintptr_t pointers[LEITZ_LM_OPEN_CALLBACK_SLOTS]={0};
    pointers[2]=11;pointers[4]=22;pointers[5]=33;pointers[6]=44;pointers[8]=55;
    leitz_lm_open_port *output=(leitz_lm_open_port *)(uintptr_t)0x1;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,&output)==1);
    assert(output && output->underlying_port==&handle&&output->monitor_context==&monitor);
    assert(!memcmp(output->host_table,pointers,sizeof pointers));
    assert(strcmp(t.log,"LDAUHR")==0);
    assert(t.last==output&&t.errors==0);
    free(output);
    t.n=0;t.log[0]=0;t.exists=1;
    output=(leitz_lm_open_port *)(uintptr_t)0x1;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,&output)==0);
    assert(strcmp(t.log,"LDEU")==0&&t.last_error==170&&t.errors==1);
    assert(output==(leitz_lm_open_port *)(uintptr_t)0x1);
    t.n=0;t.log[0]=0;t.exists=0;t.fail_open=1;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,&output)==0);
    assert(strcmp(t.log,"LDAUHF")==0 &&t.freed==1);
    t.n=0;t.log[0]=0;t.fail_allocate=1;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,&output)==0);
    assert(strcmp(t.log,"LDAU")==0&&t.freed==1);
    t.n=0;t.log[0]=0;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,21,&c,&output)==0);
    assert(t.last_error==3007&&t.errors==2&&strcmp(t.log,"E")==0);
    pointers[5]=0;t.n=0;t.log[0]=0;
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,&output)==0);
    assert(t.last_error==3007&&t.errors==3);
    assert(leitz_lm_monitor2_open_port_ex(usb,&monitor,pointers,22,&c,NULL)==0);
    puts("Original LeitzLm OpenPortEx required callbacks, duplicate and host-open cases: PASS");
    return 0;
}
