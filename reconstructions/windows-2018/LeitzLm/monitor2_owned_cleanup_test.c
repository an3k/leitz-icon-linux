#include "monitor2_owned_cleanup.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char sequence[64]; size_t n;
    int deletes,closes,free_jobs,free_ports,free_names,free_monitors;
    void *printer,*event,*monitor,*names[3],*ports[3],*jobs[4];
    size_t jobs_n,ports_n;
} trace;
static void step(trace *t,char c){assert(t->n+1<sizeof t->sequence);t->sequence[t->n++]=c;t->sequence[t->n]=0;}
static int32_t deletion(void *p,void *handle){trace *t=p;assert(handle==t->printer);step(t,'D');t->deletes++;return 0;}
static int32_t eventclose(void *p,void *handle){trace *t=p;assert(handle==t->event);step(t,'C');t->closes++;return -1;}
static void enter(void *p){step(p,'L');}
static void leave(void *p){step(p,'U');}
static void free_any(void *p,void *ptr){
    trace *t=p;
    if(ptr==t->monitor){step(t,'M');t->free_monitors++;t->monitor=NULL;}
    else if(ptr==NULL){step(t,'N');t->free_names++;}
    else {
        int found=0;
        for(size_t i=0;i<3;i++)if(ptr==t->names[i]){
            step(t,'N');t->free_names++;t->names[i]=NULL;found=1;break;
        }
        if(!found)for(size_t i=0;i<3;i++)if(ptr==t->ports[i]){
            step(t,'P');t->free_ports++;t->ports[i]=NULL;found=1;break;
        }
        if(!found)for(size_t i=0;i<t->jobs_n;i++)if(ptr==t->jobs[i]){
            step(t,'J');t->free_jobs++;t->jobs[i]=NULL;found=1;break;
        }
        assert(found);
    }
    free(ptr);
}
static leitz_lm_owned_port *new_port(trace *t,size_t index){
    leitz_lm_owned_port *p=calloc(1,sizeof *p);assert(p);
    t->ports[index]=p;
    p->name_allocation=malloc(8);assert(p->name_allocation);
    t->names[index]=p->name_allocation;
    return p;
}
static leitz_lm_document_job *new_job(trace *t){
    leitz_lm_document_job *j=calloc(1,sizeof *j);assert(j);
    assert(t->jobs_n<4);t->jobs[t->jobs_n++]=j;return j;
}
int main(void){
    trace t={0};int printer=42,event=7;
    t.printer=&printer;t.event=&event;
    const leitz_lm_owned_callbacks cb={deletion,eventclose,enter,leave,free_any};
    leitz_lm_owned_registry reg={0};
    leitz_lm_owned_port *a=new_port(&t,0),*b=new_port(&t,1);
    a->printer_for_delete=&printer;a->event_handle=&event;
    leitz_lm_document_job *j1=new_job(&t),*j2=new_job(&t);
    j1->next=j2;a->jobs=j1;
    a->next=b;reg.head=a;
    assert(leitz_lm_release_owned_port(&reg,a,&cb,&t)==0);
    assert(reg.head==b && t.deletes==1 && t.closes==1 && t.free_jobs==2);
    assert(strcmp(t.sequence,"DCLJJUNP")==0);
    t.n=0;t.sequence[0]=0;
    b->event_handle=&event;
    assert(leitz_lm_release_owned_port(&reg,b,&cb,&t)==0);
    assert(reg.head==NULL && strcmp(t.sequence,"CLUNP")==0);
    assert(t.free_ports==2 && t.free_names==2);
    /* Shutdown must invoke same release path without host ClosePort. */
    t.n=0;t.sequence[0]=0;
    leitz_lm_owned_port *c=new_port(&t,2);
    reg.head=c;c->printer_for_delete=&printer;
    c->jobs=new_job(&t);
    reg.monitor_allocation=malloc(32);assert(reg.monitor_allocation);
    t.monitor=reg.monitor_allocation;
    assert(leitz_lm_shutdown_owned_monitor(&reg,&cb,&t)==0);
    assert(reg.head==NULL && reg.monitor_allocation==NULL);
    assert(strcmp(t.sequence,"DLJUNPM")==0);
    assert(t.free_monitors==1 && t.free_ports==3 && t.free_jobs==3);
    assert(leitz_lm_shutdown_owned_monitor(&reg,&cb,&t)==-1);
    assert(leitz_lm_release_owned_port(NULL,NULL,&cb,&t)==-1);
    assert(leitz_lm_release_owned_port(&reg,NULL,&cb,&t)==-1);
    assert(leitz_lm_release_owned_port(&reg,c,NULL,&t)==-1);
    assert(leitz_lm_shutdown_owned_monitor(NULL,&cb,&t)==-1);
    puts("Windows OEM LeitzLm port release/Shutdown original cleanup order: PASS");
    return 0;
}
