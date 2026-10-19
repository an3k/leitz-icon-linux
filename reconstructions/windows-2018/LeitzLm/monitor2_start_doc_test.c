#include "monitor2_start_doc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char steps[64];size_t len;int errors,opens,fail_open,fail_start,failed_frees;
    void *event,*port,*printer_handle,*docinfo;
    uint32_t jobid,level;leitz_lm_document_job *allocated;
} trace;
static void mark(trace *t,char c){assert(t->len+1<sizeof t->steps);t->steps[t->len++]=c;t->steps[t->len]=0;}
static int32_t wait_fn(void *ctx,void *event,uint32_t ms){trace *t=ctx;assert(t->event==event&&ms==60000u);mark(t,'W');return -8;}
static int32_t reset_fn(void *ctx,void *event){trace *t=ctx;assert(t->event==event);mark(t,'R');return 0;}
static leitz_lm_document_job *alloc_fn(void *ctx,const uint16_t *name,size_t len){trace *t=ctx;assert(name&&len==3&&name[0]=='L'&&name[1]=='M'&&name[2]=='1');mark(t,'A');leitz_lm_document_job *j=malloc(sizeof *j);assert(j);t->allocated=j;return j;}
static int32_t open_fn(void *ctx,const uint16_t *name,void **handle){trace *t=ctx;assert(name&&handle);mark(t,'O');t->opens++;if(t->fail_open)return 0;*handle=t->printer_handle;return 1;}
static int32_t delegate(void *ctx,void *port,const uint16_t *name,uint32_t job,uint32_t level,const void *info){trace *t=ctx;assert(port==t->port && name && job==t->jobid && level==t->level&&info==t->docinfo);mark(t,'S');return t->fail_start?0:1;}
static void failed_free(void *ctx,leitz_lm_document_job *job){trace *t=ctx;assert(job==t->allocated);mark(t,'F');t->failed_frees++;free(job);t->allocated=NULL;}
static void err_fn(void *ctx,uint32_t code){trace *t=ctx;assert(code==87);t->errors++;mark(t,'E');}
int main(void){
    static const uint16_t printer[]={'L','M','1',0};
    int ev=1,p=2,ph=3,doc=4;
    trace t={.event=&ev,.port=&p,.printer_handle=&ph,.docinfo=&doc,.jobid=123,.level=2};
    leitz_lm_document_port port={.context=&t,.underlying_port=&p,.document_event=&ev};
    const leitz_lm_start_doc_callbacks cb={wait_fn,reset_fn,alloc_fn,open_fn,delegate,err_fn,failed_free};
    assert(leitz_lm_monitor2_start_doc(&port,printer,4,123,2,&doc,&cb)==1);
    assert(strcmp(t.steps,"WRAOS")==0 && t.errors==0 && t.opens==1);
    assert(port.jobs==t.allocated&&port.jobs->next==NULL&&port.jobs->printer_handle==&ph&&port.jobs->job_id==123);
    leitz_lm_document_job *j1=port.jobs;
    t.fail_start=1;t.len=0;t.steps[0]=0;
    assert(leitz_lm_monitor2_start_doc(&port,printer,4,123,2,&doc,&cb)==0);
    assert(strcmp(t.steps,"WRAOS")==0 && port.jobs!=j1 && port.jobs->next==j1);
    leitz_lm_document_job *j2=port.jobs;
    free(j2);free(j1);port.jobs=NULL;t.allocated=NULL;
    t.fail_open=1;t.len=0;t.steps[0]=0;
    assert(leitz_lm_monitor2_start_doc(&port,printer,4,123,2,&doc,&cb)==0);
    assert(strcmp(t.steps,"WRAOF")==0&&t.failed_frees==1&&port.jobs==NULL);
    t.len=0;t.steps[0]=0;
    assert(leitz_lm_monitor2_start_doc(&port,printer,3,123,2,&doc,&cb)==0);
    assert(strcmp(t.steps,"E")==0 && t.errors==1);
    t.len=0;t.steps[0]=0;
    assert(leitz_lm_monitor2_start_doc(&port,printer,4,123,2,NULL,&cb)==0);
    assert(t.errors==2 && strcmp(t.steps,"E")==0);
    assert(leitz_lm_monitor2_start_doc(NULL,printer,4,123,2,&doc,&cb)==0);
    assert(t.errors==2);
    t.len=0;t.steps[0]=0;
    assert(leitz_lm_monitor2_start_doc(&port,NULL,4,123,2,&doc,&cb)==0);
    assert(t.errors==3&&strcmp(t.steps,"E")==0);
    static const uint16_t empty[]={0};
    assert(leitz_lm_monitor2_start_doc(&port,empty,1,123,2,&doc,&cb)==0);
    assert(t.errors==4);
    puts("Original LeitzLm StartDocPort synchronization, job registration and host return: PASS");
    return 0;
}
