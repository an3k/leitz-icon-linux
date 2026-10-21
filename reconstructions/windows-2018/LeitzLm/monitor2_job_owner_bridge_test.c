#include "monitor2_job_owner_bridge.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    leitz_lm_owned_registry registry;
    leitz_lm_owned_port *owner;
    unsigned jobs_created,jobs_completed,jobs_freed,owned_jobs_freed;
    unsigned events,waits,resets,setjobs,closes,close_events,host_ends;
    unsigned releases,errors,allocations,deallocations;
    unsigned host_starts;
    int start_failure;
    uint32_t last_error;
    int event,device,printer,doc;
    char trace[80];size_t pos;
} fixture;
static void mark(fixture *f,char ch){assert(f->pos+1<sizeof f->trace);f->trace[f->pos++]=ch;f->trace[f->pos]=0;}
static int32_t wait_for(void *context,void *event,uint32_t milliseconds){fixture *f=context;
 assert(event==&f->event && milliseconds==60000u);f->waits++;mark(f,'W');return 0;}
static int32_t reset(void *context,void *event){fixture*f=context;assert(event==&f->event);f->resets++;mark(f,'R');return 1;}
static leitz_lm_document_job *new_job(void *context,const uint16_t *name,size_t length){fixture*f=context;
 assert(name[0]=='L' && length==5);f->jobs_created++;mark(f,'A');
 leitz_lm_document_job *p=malloc(sizeof *p);assert(p);f->allocations++;return p;}
static int32_t open_printer(void *context,const uint16_t *name,void **result){fixture*f=context;
 assert(name[0]=='L' && result);*result=&f->printer;mark(f,'O');return 1;}
static int32_t start_host(void *context,void *port,const uint16_t *name,uint32_t id,uint32_t level,const void *info){fixture*f=context;
 assert(port==&f->device && name && id && level==1 && info==&f->doc);
 f->host_starts++;mark(f,'S');return f->start_failure?0:1;}
static void job_free(void *context,leitz_lm_document_job *job){fixture*f=context;
 assert(job);f->jobs_freed++;mark(f,'F');free(job);f->deallocations++;}
static void error(void *context,uint32_t code){fixture*f=context;f->last_error=code;f->errors++;}
static int32_t event_signal(void *context,void *handle){fixture*f=context;assert(handle==&f->event);f->events++;mark(f,'E');return 1;}
static int32_t doc_end(void *context,void *port){fixture*f=context;assert(port==&f->device);f->host_ends++;mark(f,'D');return 1;}
static int32_t setjob(void *context,void *printer,uint32_t id,uint32_t level,const void *info,uint32_t cmd){fixture*f=context;
 assert(printer==&f->printer && id && level==0 && !info && cmd==7);
 f->setjobs++;f->jobs_completed++;mark(f,'J');return 0;}
static int32_t host_close(void *context,void *port){fixture*f=context;assert(port==&f->device);f->closes++;mark(f,'C');return 0;}
static int32_t delete_printer(void *context,void *h){(void)context;(void)h;assert(!"never call real DeletePrinter");return 0;}
static int32_t close_event(void *context,void *h){fixture*f=context;assert(h==&f->event);f->close_events++;mark(f,'V');return 1;}
static void lock(void *context){(void)context;}
static void unlock(void *context){(void)context;}
static void free_any(void *context,void *pointer){fixture*f=context;
 if(pointer==f->owner){f->releases++;f->owner=NULL;mark(f,'P');}
 else if(pointer){/* Job nodes and name are separately accounted. */
  /* Name allocation is a small fixed-size block in this test. */
  f->owned_jobs_freed++;mark(f,'N');
 }
 free(pointer);f->deallocations++;
}
static void release_port(void *context){fixture*f=context;
 const leitz_lm_owned_callbacks cb={delete_printer,close_event,lock,unlock,free_any};
 assert(f->owner&&f->registry.head==f->owner);
 assert(leitz_lm_release_owned_port(&f->registry,f->owner,&cb,f)==0);
}
static void setup(fixture *f,leitz_lm_document_port *document,leitz_lm_job_owner_bridge *bridge){
 f->owner=calloc(1,sizeof *f->owner);assert(f->owner);f->allocations++;
 f->owner->name_allocation=malloc(16);assert(f->owner->name_allocation);f->allocations++;
 f->owner->event_handle=&f->event;
 f->registry.head=f->owner;
 *document=(leitz_lm_document_port){.context=f,.underlying_port=&f->device,
 .document_event=&f->event,.end_document=doc_end,.close_port=host_close,
 .signal_event=event_signal,.set_job=setjob,.release_job=job_free,
 .release_port=release_port};
 *bridge=(leitz_lm_job_owner_bridge){f->owner,document};
}
static const leitz_lm_start_doc_callbacks start_callbacks={
 wait_for,reset,new_job,open_printer,start_host,error,job_free};
static const uint16_t name[]={'L','e','i','t','z',0};
static void completed_jobs(void){
 fixture f={0};leitz_lm_document_port document;leitz_lm_job_owner_bridge bridge;
 setup(&f,&document,&bridge);
 assert(leitz_lm_job_owner_views_match(&bridge));
 assert(leitz_lm_job_owner_start(&bridge,name,6,12u,1,&f.doc,&start_callbacks)==1);
 assert(f.owner->jobs==document.jobs && document.jobs->job_id==12);
 assert(leitz_lm_job_owner_start(&bridge,name,6,13u,1,&f.doc,&start_callbacks)==1);
 assert(f.owner->jobs==document.jobs && document.jobs->next->job_id==12);
 assert(leitz_lm_job_owner_end(&bridge)==1);
 assert(f.owner->jobs==NULL && document.jobs==NULL);
 assert(f.jobs_created==2 && f.jobs_freed==2 && f.jobs_completed==2 && f.setjobs==2);
 assert(f.host_starts==2 && f.waits==2 && f.resets==2 && f.events==1);
 assert(leitz_lm_job_owner_close(&bridge)==1);
 assert(bridge.owned==NULL && document.jobs==NULL && f.registry.head==NULL);
 assert(f.releases==1 && f.close_events==1 && f.closes==1);
 assert(f.allocations==4 && f.deallocations==4);
 assert(!leitz_lm_job_owner_views_match(&bridge));
 assert(leitz_lm_job_owner_end(&bridge)==-1);
}
static void failed_host_keeps_job_until_port_release(void){
 fixture f={0};leitz_lm_document_port document;leitz_lm_job_owner_bridge bridge;
 setup(&f,&document,&bridge);f.start_failure=1;
 assert(leitz_lm_job_owner_start(&bridge,name,6,37u,1,&f.doc,&start_callbacks)==0);
 assert(f.owner->jobs && f.owner->jobs==document.jobs);
 assert(f.jobs_created==1 && f.jobs_freed==0);
 /* Prove invariant enforcement, not silent overwriting one list. */
 document.jobs=NULL;
 assert(!leitz_lm_job_owner_views_match(&bridge));
 assert(leitz_lm_job_owner_end(&bridge)==-1);
 assert(leitz_lm_job_owner_close(&bridge)==-1);
 assert(f.owner->jobs && f.jobs_freed==0 && f.releases==0);
 document.jobs=f.owner->jobs;
 /* Close should release this still-pending job via existing destructor,
  * without a second EndDoc/free of the same allocation. */
 assert(leitz_lm_job_owner_close(&bridge)==1);
 assert(bridge.owned==NULL && document.jobs==NULL && f.registry.head==NULL);
 assert(f.jobs_freed==0 && f.owned_jobs_freed==2); /* job + name */
 assert(f.releases==1 && f.deallocations==3 && f.allocations==3);
 assert(f.closes==1 && f.close_events==1);
}
static void absent_release_is_safe(void){
 fixture f={0};leitz_lm_document_port document;leitz_lm_job_owner_bridge bridge;
 setup(&f,&document,&bridge);
 document.release_port=NULL;
 assert(leitz_lm_job_owner_close(&bridge)==-1);
 assert(f.owner && bridge.owned==f.owner && f.registry.head==f.owner);
 document.release_port=release_port;
 assert(leitz_lm_job_owner_close(&bridge)==1);
 assert(!f.owner && !bridge.owned);
}
int main(void){completed_jobs();failed_host_keeps_job_until_port_release();absent_release_is_safe();
 puts("Original Windows language-monitor shared job list: start/end/close ownership bridge PASS");return 0;}
