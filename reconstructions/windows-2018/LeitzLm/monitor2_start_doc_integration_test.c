#include "monitor2_start_doc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {char events[32];size_t n;uint32_t id;unsigned freed;leitz_lm_document_job *last;int h,e;} state;
static void step(state *s,char c){assert(s->n<sizeof s->events-1);s->events[s->n++]=c;s->events[s->n]=0;}
static int32_t wait_cb(void *v,void *e,uint32_t ms){state*s=v;assert(e==&s->e&&ms==60000);step(s,'W');return 0;}
static int32_t reset_cb(void *v,void *e){state*s=v;assert(e==&s->e);step(s,'R');return 0;}
static leitz_lm_document_job *alloc_cb(void *v,const uint16_t *name,size_t n){state*s=v;assert(n==3&&name[2]=='N');step(s,'A');leitz_lm_document_job *j=malloc(sizeof *j);assert(j);return j;}
static int32_t open_cb(void *v,const uint16_t *name,void **h){state*s=v;assert(name[0]=='I');step(s,'O');*h=&s->h;return 1;}
static int32_t start_cb(void *v,void *port,const uint16_t *name,uint32_t id,uint32_t level,const void *di){state*s=v;assert(port==&s->h&&name[1]=='C'&&id==s->id&&level==0&&di==&s->e);step(s,'S');return 1;}
static void release_failed(void *v,leitz_lm_document_job *j){(void)v;free(j);assert(0);}
static int32_t event_cb(void *v,void *e){state*s=v;assert(e==&s->e);step(s,'E');return 1;}
static int32_t end_cb(void *v,void *p){state*s=v;assert(p==&s->h);step(s,'D');return 1;}
static int32_t setjob_cb(void *v,void *p,uint32_t id,uint32_t level,const void *info,uint32_t cmd){state*s=v;assert(p==&s->h && id==s->id && level==0 && !info && cmd==7);step(s,'J');return 0;}
static void freejob_cb(void *v,leitz_lm_document_job *job){state*s=v;step(s,'F');s->freed++;free(job);}
int main(void){
    state s={.id=47};static const uint16_t printer[]={'I','C','N',0};
    leitz_lm_document_port p={.context=&s,.underlying_port=&s.h,
       .document_event=&s.e,.signal_event=event_cb,.end_document=end_cb,
       .set_job=setjob_cb,.release_job=freejob_cb};
    leitz_lm_start_doc_callbacks callbacks={.wait=wait_cb,.reset=reset_cb,.allocate_job=alloc_cb,
      .open_printer=open_cb,.start_document=start_cb,.release_failed_job=release_failed};
    assert(leitz_lm_monitor2_start_doc(&p,printer,4,s.id,0,&s.e,&callbacks)==1);
    assert(p.jobs&&p.jobs->job_id==47&&p.jobs->printer_handle==&s.h);
    assert(leitz_lm_monitor2_end_doc(&p)==1);
    assert(!p.jobs&&s.freed==1&&strcmp(s.events,"WRAOSEDJF")==0);
    puts("Original StartDocPort -> existing EndDocPort SetJobW(7) and cleanup: PASS");
    return 0;
}
