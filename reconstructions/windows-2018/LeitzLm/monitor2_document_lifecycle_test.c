#include "monitor2_document_lifecycle.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Observe full cross-callback ordering and preserve result conditions. */
typedef struct {
    unsigned calls, events, ends, setjobs, jobfrees, closes, portfrees;
    char order[32];
    void *event, *underlying, *p1, *p2;
    uint32_t jobid[2];
    int32_t end_result, close_result;
    int32_t event_result, job_result;
    leitz_lm_document_job *j1,*j2;
} trace;
static void record(trace *t,char type) {
    assert(t->calls < sizeof t->order-1);
    t->order[t->calls++]=type;t->order[t->calls]=0;
}
static int32_t event_fn(void *ctx,void *value) {
    trace *t=ctx;assert(value==t->event);record(t,'E');++t->events;
    return t->event_result;
}
static int32_t end_fn(void *ctx,void *value) {
    trace *t=ctx;assert(value==t->underlying);record(t,'D');++t->ends;
    return t->end_result;
}
static int32_t set_job_fn(void *ctx,void *printer,uint32_t id,uint32_t level,
                           const void *info,uint32_t command) {
    trace *t=ctx;assert(level==0u && info==NULL && command==7u);
    assert(t->setjobs<2);
    assert(printer==(t->setjobs ? t->p2 : t->p1));
    assert(id==t->jobid[t->setjobs]);
    record(t,'J');++t->setjobs;
    return t->job_result;
}
static void free_job_fn(void *ctx,leitz_lm_document_job *job) {
    trace *t=ctx;assert(job==(t->jobfrees ? t->j2 : t->j1));
    record(t,'F');++t->jobfrees;
}
static int32_t close_fn(void *ctx,void *value) {
    trace *t=ctx;assert(value==t->underlying);record(t,'C');++t->closes;
    return t->close_result;
}
static void free_port_fn(void *ctx) {
    trace *t=ctx;record(t,'P');++t->portfrees;
}
int main(void)
{
    int event=1,handle=2,p1=3,p2=4;
    leitz_lm_document_job job2={NULL,&p2,77};
    leitz_lm_document_job job1={&job2,&p1,12};
    trace t={.event=&event,.underlying=&handle,.p1=&p1,.p2=&p2,
             .jobid={12,77},.j1=&job1,.j2=&job2,
             .end_result=-17,.close_result=0,.event_result=0,.job_result=0};
    leitz_lm_document_port port={.context=&t,.underlying_port=&handle,
      .document_event=&event,.end_document=end_fn,.close_port=close_fn,
      .signal_event=event_fn,.set_job=set_job_fn,
      .release_job=free_job_fn,.release_port=free_port_fn,.jobs=&job1};
    assert(leitz_lm_monitor2_end_doc(&port)==-17);
    assert(port.jobs==NULL && t.events==1 && t.ends==1);
    assert(t.setjobs==2 && t.jobfrees==2);
    assert(t.calls==6);
    assert(t.order[0]=='E'&&t.order[1]=='D'&&t.order[2]=='J'&&
           t.order[3]=='J'&&t.order[4]=='F'&&t.order[5]=='F');
    assert(leitz_lm_monitor2_close_port(&port)==1);
    assert(t.closes==1 && t.portfrees==1 && t.calls==8);
    assert(t.order[6]=='C'&&t.order[7]=='P');

    /* No jobs: same event -> underlying doc completion, preserving TRUE. */
    t.calls=t.events=t.ends=t.setjobs=t.jobfrees=t.closes=t.portfrees=0;
    port.jobs=NULL;t.end_result=1;t.event_result=-42;
    assert(leitz_lm_monitor2_end_doc(&port)==1);
    assert(t.calls==2&&t.order[0]=='E'&&t.order[1]=='D');

    /* SetJobW failure does not override delegate return or suppress frees. */
    port.jobs=&job1;t.calls=t.events=t.ends=t.setjobs=t.jobfrees=0;
    t.end_result=0;t.job_result=0;
    assert(leitz_lm_monitor2_end_doc(&port)==0);
    assert(t.setjobs==2&&t.jobfrees==2&&port.jobs==NULL);

    /* ClosePort absent: OEM skips delegate but still frees port. */
    t.calls=t.portfrees=t.closes=0;port.close_port=NULL;
    assert(leitz_lm_monitor2_close_port(&port)==1);
    assert(t.calls==1&&t.order[0]=='P'&&t.portfrees==1&&t.closes==0);
    assert(leitz_lm_monitor2_close_port(NULL)==0);
    port.release_port=NULL;
    assert(leitz_lm_monitor2_close_port(&port)==0&&t.portfrees==1);

    /* Safe model rejects callbacks it cannot execute without mutation. */
    port.release_port=free_port_fn;port.jobs=&job1;
    port.release_job=NULL;t.calls=0;
    assert(leitz_lm_monitor2_end_doc(&port)==0);
    assert(t.calls==0&&port.jobs==&job1);
    port.release_job=free_job_fn;port.set_job=NULL;
    assert(leitz_lm_monitor2_end_doc(&port)==0);
    assert(t.calls==0&&port.jobs==&job1);
    port.set_job=set_job_fn;port.signal_event=NULL;
    assert(leitz_lm_monitor2_end_doc(&port)==0);
    assert(t.calls==0);
    assert(leitz_lm_monitor2_end_doc(NULL)==0);
    puts("Original LeitzLm x86/x64 EndDocPort and ClosePort lifecycle: PASS");
    return 0;
}
