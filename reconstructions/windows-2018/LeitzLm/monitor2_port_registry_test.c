#include "monitor2_port_registry.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

typedef struct {char seq[100];int at,alloc,free_calls,events,close_events,lock,fail_on_alloc,fail_event;size_t sizes[8];void *event;int event_manual,event_initial,comparisons;} state;
static void mark(state*s,char c){assert(s->at < 99);s->seq[s->at++]=c;s->seq[s->at]=0;}
static void *allocate(void *ctx,size_t n){state*s=ctx;mark(s,'A');s->sizes[s->alloc++]=n;if(s->alloc==s->fail_on_alloc)return NULL;void*p=malloc(n);assert(p);memset(p,0xa5,n);return p;}
static void deallocate(void *ctx,void *ptr){state*s=ctx;mark(s,'F');s->free_calls++;free(ptr);}
static void *create_event(void *ctx,int manual,int initial){state*s=ctx;mark(s,'E');s->events++;s->event_manual=manual;s->event_initial=initial;return s->fail_event?NULL:s->event;}
static int32_t cmp(void *ctx,const uint16_t*a,const uint16_t*b){state*s=ctx;mark(s,'C');s->comparisons++;for(size_t i=0;i<100;i++){int ca=a[i],cb=b[i];if(ca>='a'&&ca<='z')ca-=32;if(cb>='a'&&cb<='z')cb-=32;if(ca!=cb)return 1;if(!ca)return 0;}assert(0);return 1;}
static int32_t no_delete(void *ctx,void *h){(void)ctx;(void)h;assert(0);return 0;}
static int32_t close_event(void *ctx,void *h){state*s=ctx;assert(h==s->event);mark(s,'H');s->close_events++;return 1;}
static void job_lock(void *ctx){state*s=ctx;mark(s,'L');s->lock++;}
static void job_unlock(void *ctx){state*s=ctx;mark(s,'U');s->lock--;}
int main(void){
 static const uint16_t one[]={ 'L','e','i','t','z','U','S','B',0 };
 static const uint16_t mixed[]={ 'l','E','I','T','Z','u','s','b',0 };
 static const uint16_t two[]={ 'L','e','i','t','z','T','C','P',0 };
 int event=89;state s={0};s.event=&event;
 leitz_lm_registry_services svc={&s,cmp,allocate,deallocate,create_event};
 leitz_lm_owned_registry reg={0};
 assert(!leitz_lm_native_find_port(&reg,one,&svc));
 assert(!leitz_lm_native_find_port(&reg,NULL,&svc));
 leitz_lm_native_port *p=leitz_lm_native_create_port(&reg,one,9,LEITZ_LM_ARCH_X86,&svc);
 assert(p && reg.head==&p->owned && p->owned.next==NULL);
 assert(strcmp(s.seq,"AAE")==0 && s.alloc==2 && s.sizes[0]==0x478 && s.sizes[1]==18);
 assert(s.events==1 && s.event_manual==1 && s.event_initial==1);
 assert(p->owned.event_handle==&event && p->allocation_bytes==0x478 && p->startup_flag==0);
 assert(!memcmp(p->owned.name_allocation,one,sizeof one));
 for(size_t i=0;i<sizeof p->port_scratch;i++)assert(p->port_scratch[i]==0);
 assert(leitz_lm_native_find_port(&reg,mixed,&svc)==&p->owned && s.comparisons==1);
 assert(!leitz_lm_native_find_port(&reg,two,&svc) && s.comparisons==2);
 s.at=0;s.seq[0]=0;s.fail_event=1;
 leitz_lm_native_port *q=leitz_lm_native_create_port(&reg,two,9,LEITZ_LM_ARCH_X64,&svc);
 assert(q && reg.head==&q->owned && q->owned.next==&p->owned);
 assert(s.sizes[2]==0x4f0 && s.sizes[3]==18 && !q->owned.event_handle);
 assert(q->allocation_bytes==0x4f0 && strcmp(s.seq,"AAE")==0);
 assert(leitz_lm_native_find_port(&reg,one,&svc)==&p->owned);
 s.at=0;s.seq[0]=0;s.fail_on_alloc=6; /* next port allocation succeeds, name allocation fails */
 leitz_lm_owned_port *old=reg.head;
 assert(!leitz_lm_native_create_port(&reg,one,9,LEITZ_LM_ARCH_X86,&svc));
 assert(strcmp(s.seq,"AAF")==0 && reg.head==old && s.free_calls==1);
 s.fail_on_alloc=0;
 assert(!leitz_lm_native_create_port(&reg,one,8,LEITZ_LM_ARCH_X86,&svc));
 assert(!leitz_lm_native_create_port(&reg,one,9,(leitz_win2018_monitor2_arch)8,&svc));
 assert(!leitz_lm_native_create_port(&reg,NULL,9,LEITZ_LM_ARCH_X86,&svc));
 assert(!leitz_lm_native_create_port(NULL,one,9,LEITZ_LM_ARCH_X86,&svc));
 static const uint16_t empty[]={0};
 assert(!leitz_lm_native_create_port(&reg,empty,1,LEITZ_LM_ARCH_X86,&svc));
 assert(!leitz_lm_native_find_port(&reg,empty,&svc));
 leitz_lm_owned_callbacks cleanup={no_delete,close_event,job_lock,job_unlock,deallocate};
 assert(leitz_lm_release_owned_port(&reg,&p->owned,&cleanup,&s)==0);
 assert(reg.head==&q->owned && s.close_events==1 && s.lock==0);
 assert(leitz_lm_release_owned_port(&reg,&q->owned,&cleanup,&s)==0);
 assert(reg.head==NULL && s.close_events==1 && s.lock==0);
 puts("Original Windows LeitzLm native port allocation and case-insensitive registry: PASS");
 return 0;
}
