/* Integration-only regression of EXISTING portable Windows 2018 LeitzLm
 * reconstruction modules. No Win32 ABI, physical printer I/O or OEM DLL
 * execution is claimed. All side effects use deterministic callbacks.
 * Port ownership across the independent public C models is supplied here
 * by the fixture; this is an explicit bridge test, not native ABI glue. */
#include "monitor2_native_map.h"
#include "monitor2_port_registry.h"
#include "monitor2_open_port.h"
#include "monitor2_port_io.h"
#include "monitor2_start_doc.h"
#include "monitor2_document_lifecycle.h"
#include "monitor2_owned_cleanup.h"
#include "status_dispatch.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    leitz_lm_owned_registry registry;
    leitz_lm_registry_services registry_api;
    leitz_lm_owned_callbacks cleanup;
    leitz_lm_native_port *native;
    leitz_lm_open_port *opened;
    leitz_win2018_monitor2_arch arch;
    int monitor_token, underlying, printer_token, event_token, doc_info;
    int registry_lock, job_lock, opens, closes, event_creates, event_closes;
    int allocations, frees, jobs_allocated, jobs_freed, writes, reads;
    int wait_calls, reset_calls, end_events, job_cancel_calls;
    int host_starts, host_ends, error_count, record_count;
    uint32_t last_error, last_job_id;
    int fail_host_open;
    char log[128];size_t pos;
    uint8_t last_write[4];
} fixture;
static void step(fixture *f,char c){assert(f->pos+1<sizeof f->log);f->log[f->pos++]=c;f->log[f->pos]=0;}
static void *mem_alloc(void *v,size_t bytes){fixture*f=v;f->allocations++;return malloc(bytes);}
static void mem_free(void *v,void *ptr){fixture*f=v;f->frees++;free(ptr);}
/* Name comparison here covers only the ASCII test fixture. Production
 * uses the native lstrcmpiW callback from the existing registry model. */
static int32_t compare(void *v,const uint16_t *a,const uint16_t *b){(void)v;
 for(size_t i=0;i<32;i++){
  unsigned x=a[i],y=b[i];
  if(x>='a'&&x<='z')x-=32;
  if(y>='a'&&y<='z')y-=32;
  if(x!=y)return 1;
  if(!x)return 0;
 }return 1;
}
static void *create_event(void *v,int manual,int initial){fixture*f=v;
 assert(manual==1&&initial==1);f->event_creates++;step(f,'V');return &f->event_token;}
static void lock_registry(void *v){fixture*f=v;assert(!f->registry_lock++);step(f,'L');}
static void unlock_registry(void *v){fixture*f=v;assert(!--f->registry_lock);step(f,'U');}
static void lock_jobs(void *v){fixture*f=v;assert(!f->job_lock++);step(f,'K');}
static void unlock_jobs(void *v){fixture*f=v;assert(!--f->job_lock);step(f,'k');}
static int32_t delete_printer(void *v,void *h){(void)v;(void)h;assert(!"unexpected physical DeletePrinter");return 0;}
static int32_t close_event(void *v,void *h){fixture*f=v;assert(h==&f->event_token);f->event_closes++;step(f,'X');return 1;}
static int32_t port_exists(void *v,const uint16_t *name){fixture*f=v;
 assert(f->registry_lock==1);return leitz_lm_native_find_port(&f->registry,name,&f->registry_api)!=NULL;}
static leitz_lm_open_port *allocate_port(void *v,const uint16_t *name){fixture*f=v;
 assert(f->registry_lock==1&&f->native==NULL);
 f->native=leitz_lm_native_create_port(&f->registry,name,32,f->arch,&f->registry_api);
 if(!f->native)return NULL;
 f->opened=calloc(1,sizeof *f->opened);assert(f->opened);
 return f->opened;
}
static void release_native(fixture*f){
 assert(f->native);
 assert(leitz_lm_release_owned_port(&f->registry,&f->native->owned,&f->cleanup,f)==0);
 f->native=NULL;
}
static void release_failed_open(void *v,leitz_lm_open_port *p){fixture*f=v;
 assert(f->opened==p);release_native(f);free(p);f->opened=NULL;step(f,'F');}
static int32_t host_open(void *v,void *monitor,const uint16_t *name,void **output){fixture*f=v;
 assert(monitor==&f->monitor_token&&name&&output);f->opens++;step(f,'O');
 if(f->fail_host_open)return 0;
 *output=&f->underlying;return 1;
}
static void error(void *v,uint32_t code){fixture*f=v;f->last_error=code;f->error_count++;step(f,'!');}
static void record(void *v,leitz_lm_open_port *port){fixture*f=v;assert(f->opened==port);f->record_count++;step(f,'P');}
static int32_t write_cb(void *v,void *h,const uint8_t *bytes,uint32_t n,uint32_t *done){fixture*f=v;
 assert(h==&f->underlying&&n<=sizeof f->last_write&&done);
 memcpy(f->last_write,bytes,n);*done=n;f->writes++;step(f,'W');return 1;
}
static int32_t read_cb(void *v,void *h,uint8_t *buf,uint32_t n,uint32_t *done){fixture*f=v;
 assert(h==&f->underlying&&n>=2&&buf&&done);
 buf[0]='O';buf[1]='K';*done=2;f->reads++;step(f,'R');return 1;
}
static int32_t exchange(void *v,const uint8_t *cmd,size_t n,uint8_t *reply,size_t cap,size_t *length){
 leitz_win2018_monitor2_port *io=v;
 uint32_t written=0,read=0;
 if(!leitz_win2018_monitor2_write(io,cmd,(uint32_t)n,&written)||written!=n)return -1;
 if(!leitz_win2018_monitor2_read(io,reply,(uint32_t)cap,&read))return -1;
 *length=read;return 0;
}
static int32_t wait_event(void *v,void *handle,uint32_t time){fixture*f=v;
 assert(handle==&f->event_token&&time==60000);f->wait_calls++;step(f,'w');return 0;
}
static int32_t reset_event(void *v,void *handle){fixture*f=v;
 assert(handle==&f->event_token);f->reset_calls++;step(f,'r');return 1;
}
static leitz_lm_document_job *allocate_job(void *v,const uint16_t *name,size_t len){fixture*f=v;
 assert(name&&len==5);f->jobs_allocated++;step(f,'J');return malloc(sizeof(leitz_lm_document_job));
}
static int32_t open_printer(void *v,const uint16_t *name,void **out){fixture*f=v;
 assert(name&&out);*out=&f->printer_token;step(f,'Q');return 1;
}
static int32_t start_host(void *v,void *port,const uint16_t *name,uint32_t id,uint32_t level,const void *info){fixture*f=v;
 assert(port==&f->underlying&&name&&id==2018u&&level==1u&&info==&f->doc_info);
 f->host_starts++;step(f,'S');return 1;
}
static int32_t signal(void *v,void *handle){fixture*f=v;assert(handle==&f->event_token);f->end_events++;step(f,'E');return 1;}
static int32_t end_host(void *v,void *port){fixture*f=v;assert(port==&f->underlying);f->host_ends++;step(f,'D');return 1;}
static int32_t set_job(void *v,void *printer,uint32_t id,uint32_t level,const void *info,uint32_t command){fixture*f=v;
 assert(printer==&f->printer_token&&id==2018u&&level==0&&!info&&command==7u);
 f->job_cancel_calls++;f->last_job_id=id;step(f,'C');return 0; /* OEM ignores failed SetJobW */
}
static void release_job(void *v,leitz_lm_document_job *job){fixture*f=v;
 f->jobs_freed++;step(f,'j');free(job);}
static int32_t close_host(void *v,void *port){fixture*f=v;assert(port==&f->underlying);f->closes++;step(f,'H');return 0;}
static void release_session(void *v){fixture*f=v;
 release_native(f);free(f->opened);f->opened=NULL;step(f,'T');}

static void scenario(leitz_win2018_monitor2_arch arch){
    static const uint16_t usb[]={'L','e','i','t','z','U','S','B',0};
    static const uint16_t duplicate[]={'l','E','I','T','Z','u','s','b',0};
    static const uint16_t printer[]={'L','e','i','t','z',0};
    static const uint16_t status[]={'l','e','i','t','z','_','o','e','m','_','s','t','a','t','u','s',0};
    fixture f={0};f.arch=arch;
    f.registry_api=(leitz_lm_registry_services){&f,compare,mem_alloc,mem_free,create_event};
    f.cleanup=(leitz_lm_owned_callbacks){delete_printer,close_event,lock_jobs,unlock_jobs,mem_free};
    leitz_lm_open_callbacks op={&f,lock_registry,unlock_registry,port_exists,allocate_port,
                                release_failed_open,host_open,error,record};
    uintptr_t host_table[22]={0};host_table[2]=1;host_table[4]=2;host_table[5]=3;
    host_table[6]=4;host_table[8]=5;
    leitz_lm_open_port *opened=NULL;
    assert(leitz_lm_monitor2_open_port_ex(usb,&f.monitor_token,host_table,22,&op,&opened)==1);
    assert(opened==f.opened&&opened->underlying_port==&f.underlying&&f.registry.head==&f.native->owned);
    assert(f.event_creates==1&&f.registry_lock==0&&f.record_count==1);
    assert(f.native->allocation_bytes==(arch==LEITZ_LM_ARCH_X86?0x478u:0x4f0u));
    assert(f.native->owned.event_handle==&f.event_token);
    assert(!memcmp(opened->host_table,host_table,sizeof host_table));
    leitz_lm_open_port *unchanged=opened;
    assert(!leitz_lm_monitor2_open_port_ex(duplicate,&f.monitor_token,host_table,22,&op,&unchanged));
    assert(unchanged==opened&&f.error_count==1&&f.last_error==170u);
    leitz_win2018_monitor2_port io={&f.underlying,&f,write_cb,read_cb,error};
    uint8_t data[]={0x1b,'G'};uint32_t transferred=0;
    assert(leitz_win2018_monitor2_write(&io,data,2,&transferred)==1);
    assert(transferred==2&&f.last_write[0]==0x1b&&f.last_write[1]=='G');
    uint8_t answer[8]={0};size_t response=0;
    assert(leitz_win2018_lm_dispatch(status,answer,sizeof answer,&response,exchange,&io)==1);
    assert(response==2&&!memcmp(answer,"OK",2)&&f.writes==2&&f.reads==1);
    assert(f.last_write[0]==0x1b&&f.last_write[1]=='A');
    leitz_lm_document_port document={.context=&f,.underlying_port=&f.underlying,
        .document_event=&f.event_token,.end_document=end_host,.close_port=close_host,
        .signal_event=signal,.set_job=set_job,.release_job=release_job,
        .release_port=release_session};
    leitz_lm_start_doc_callbacks start={wait_event,reset_event,allocate_job,open_printer,
                                         start_host,error,release_job};
    assert(leitz_lm_monitor2_start_doc(&document,printer,6,2018,1,&f.doc_info,&start)==1);
    assert(document.jobs&&document.jobs->job_id==2018&&document.jobs->printer_handle==&f.printer_token);
    /* The two legacy data models intentionally keep separate linked
     * fields: this test verifies the EndDoc boundary before ownership
     * cleanup and does not manufacture native C struct aliasing. */
    assert(leitz_lm_monitor2_end_doc(&document)==1);
    assert(!document.jobs&&f.jobs_allocated==1&&f.jobs_freed==1&&f.job_cancel_calls==1);
    assert(f.last_job_id==2018u&&f.wait_calls==1&&f.reset_calls==1&&f.host_starts==1);
    assert(f.host_ends==1&&f.end_events==1);
    assert(leitz_lm_monitor2_close_port(&document)==1);
    assert(f.closes==1&&f.event_closes==1&&f.registry.head==NULL&&f.native==NULL);
    assert(f.opened==NULL&&f.registry_lock==0&&f.job_lock==0);
    assert(f.allocations==2&&f.frees==2&&f.error_count==1);
    assert(f.log[f.pos-1]=='T');
}
int main(void){scenario(LEITZ_LM_ARCH_X86);scenario(LEITZ_LM_ARCH_X64);
 puts("Windows LeitzLm full-session regression: port registry -> open -> write/status -> start -> end -> cleanup (x86/x64): PASS");
 return 0;
}
