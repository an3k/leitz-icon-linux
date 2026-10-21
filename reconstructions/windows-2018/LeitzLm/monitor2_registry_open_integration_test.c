/* End-to-end portable composition of existing OEM-derived LeitzLm modules.
 * Exercises REAL registry, OpenPortEx and owned cleanup implementations.
 * No Win32 spooler handles; DeletePrinter MUST remain a test-only callback.
 */
#include "monitor2_port_registry.h"
#include "monitor2_open_port.h"
#include "monitor2_owned_cleanup.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    leitz_lm_owned_registry registry;
    leitz_lm_native_port *new_native;
    int event, monitor, underlying;
    int lock_depth, job_lock_depth, alloc_count;
    int created_events, closed_events, opened_hosts, failed_host;
    int deleted_printers, frees, duplicate_error, invalid_error;
    int host_closed, record_count;
    unsigned last_error;
} fixture;

static int32_t compare_ci(void *context,const uint16_t *a,const uint16_t *b) {
    (void)context;
    /* Only test fixtures are ASCII; real Windows lstrcmpiW remains
       a required external boundary for general Unicode names. */
    for (size_t i=0;i<32;i++) {
        uint16_t x=a[i],y=b[i];
        if(x>='a' && x<='z')x=(uint16_t)(x-32);
        if(y>='a' && y<='z')y=(uint16_t)(y-32);
        if(x!=y)return 1;
        if(!x)return 0;
    }
    assert(0 && "unterminated fixture name");
    return 1;
}
static void *alloc_native(void *ctx,size_t bytes) {
    fixture *f=ctx;f->alloc_count++;
    void *p=malloc(bytes);
    assert(p);
    memset(p,0xcc,bytes);
    return p;
}
static void free_native(void *ctx,void *p) {
    fixture *f=ctx;f->frees++;
    free(p);
}
static void *create_event(void *ctx,int manual,int signaled) {
    fixture *f=ctx;
    assert(manual==1 && signaled==1);
    f->created_events++;
    return &f->event;
}
static const leitz_lm_registry_services services = {
    NULL,compare_ci,alloc_native,free_native,create_event
};
static leitz_lm_registry_services bind_services(fixture *f) {
    leitz_lm_registry_services r=services;
    r.context=f;
    return r;
}
static int32_t deletion_guard(void *ctx,void *printer) {
    fixture *f=ctx;
    (void)printer;
    f->deleted_printers++;
    /* A real DeletePrinter invocation is forbidden in this offline test. */
    return 1;
}
static int32_t close_event(void *ctx,void *event) {
    fixture *f=ctx;assert(event==&f->event);
    f->closed_events++;
    return 1;
}
static void job_lock(void *ctx) { fixture *f=ctx;assert(++f->job_lock_depth==1); }
static void job_unlock(void *ctx) { fixture *f=ctx;assert(--f->job_lock_depth==0); }
static const leitz_lm_owned_callbacks cleanup = {
    deletion_guard, close_event, job_lock, job_unlock, free_native
};
static void enter(void *ctx) { fixture *f=ctx;assert(++f->lock_depth==1); }
static void leave(void *ctx) { fixture *f=ctx;assert(--f->lock_depth==0); }
static int32_t exists(void *ctx,const uint16_t *name) {
    fixture *f=ctx;assert(f->lock_depth==1);
    leitz_lm_registry_services s=bind_services(f);
    return leitz_lm_native_find_port(&f->registry,name,&s)!=NULL;
}
static leitz_lm_open_port *allocate_port(void *ctx,const uint16_t *name) {
    fixture *f=ctx;assert(f->lock_depth==1 && f->new_native==NULL);
    leitz_lm_registry_services s=bind_services(f);
    f->new_native=leitz_lm_native_create_port(
        &f->registry,name,32u,LEITZ_LM_ARCH_X64,&s);
    if(!f->new_native)return NULL;
    leitz_lm_open_port *view=calloc(1,sizeof *view);
    if(!view) {
        assert(leitz_lm_release_owned_port(&f->registry,
            &f->new_native->owned,&cleanup,f)==0);
        f->new_native=NULL;
    }
    return view;
}
static void release_port(void *ctx,leitz_lm_open_port *view) {
    fixture *f=ctx;assert(view && f->new_native);
    assert(leitz_lm_release_owned_port(&f->registry,
        &f->new_native->owned,&cleanup,f)==0);
    f->new_native=NULL;
    free(view);
}
static int32_t host_open(void *ctx,void *monitor,const uint16_t *name,void **underlying) {
    fixture *f=ctx;assert(monitor==&f->monitor);
    assert(name && f->lock_depth==0 && underlying);
    f->opened_hosts++;
    if(f->failed_host)return 0;
    *underlying=&f->underlying;
    return 1;
}
static void report_error(void *ctx,uint32_t code) {
    fixture *f=ctx;
    f->last_error=code;
    if(code==170)f->duplicate_error++;
    if(code==3007)f->invalid_error++;
}
static void record_port(void *ctx,leitz_lm_open_port *view) {
    fixture *f=ctx;
    assert(view->underlying_port==&f->underlying);
    f->record_count++;
}
static leitz_lm_open_callbacks bind_open(fixture *f) {
    leitz_lm_open_callbacks x={f,enter,leave,exists,allocate_port,
                               release_port,host_open,report_error,record_port};
    return x;
}
int main(void) {
    static const uint16_t original[]={ 'L','e','i','t','z','U','S','B',0 };
    static const uint16_t alias[]={ 'l','E','I','T','Z','u','s','b',0 };
    uintptr_t host_table[LEITZ_LM_OPEN_CALLBACK_SLOTS]={0};
    host_table[2]=11;host_table[4]=22;host_table[5]=33;
    host_table[6]=44;host_table[8]=55;
    fixture f={0};
    leitz_lm_open_callbacks o=bind_open(&f);
    leitz_lm_open_port *view=NULL;
    /* x64 native allocation, original UTF16 name, event, registration,
       host OpenPort and full host-table copy all exercised together. */
    assert(leitz_lm_monitor2_open_port_ex(original,&f.monitor,host_table,
                                          LEITZ_LM_OPEN_CALLBACK_SLOTS,&o,&view)==1);
    assert(view && f.new_native && f.registry.head==&f.new_native->owned);
    assert(f.new_native->allocation_bytes==0x4f0u);
    assert(f.created_events==1 && f.record_count==1 && f.opened_hosts==1);
    assert(!memcmp(view->host_table,host_table,sizeof host_table));
    assert(!memcmp(f.new_native->owned.name_allocation,original,sizeof original));
    assert(f.new_native->owned.event_handle==&f.event);
    for(size_t i=0;i<sizeof f.new_native->port_scratch;i++)
        assert(f.new_native->port_scratch[i]==0);
    /* A second, differently cased spelling must be rejected BEFORE
       allocation or host opening, using the existing registry lookup. */
    leitz_lm_open_port *ignored=NULL;
    assert(leitz_lm_monitor2_open_port_ex(alias,&f.monitor,host_table,
                  LEITZ_LM_OPEN_CALLBACK_SLOTS,&o,&ignored)==0);
    assert(!ignored && f.last_error==170 && f.duplicate_error==1);
    assert(f.opened_hosts==1 && f.created_events==1 && f.record_count==1);
    /* Release follows the EXISTING owned-port destructor. No real
       DeletePrinter call is permitted. */
    release_port(&f,view);view=NULL;
    assert(f.registry.head==NULL && f.closed_events==1);
    assert(f.deleted_printers==0 && f.lock_depth==0 && f.job_lock_depth==0);
    /* Failure of the host OpenPort unwinds the existing native registry
       and event, without publishing a handle. */
    f.failed_host=1;
    assert(leitz_lm_monitor2_open_port_ex(original,&f.monitor,host_table,
                  LEITZ_LM_OPEN_CALLBACK_SLOTS,&o,&view)==0);
    assert(!view && !f.registry.head && !f.new_native);
    assert(f.created_events==2 && f.closed_events==2 && f.record_count==1);
    /* Missing required host callback is rejected before any allocation. */
    host_table[5]=0;
    assert(leitz_lm_monitor2_open_port_ex(original,&f.monitor,host_table,
                  LEITZ_LM_OPEN_CALLBACK_SLOTS,&o,&view)==0);
    assert(f.invalid_error==1 && f.last_error==3007 && f.created_events==2);
    assert(f.registry.head==NULL && f.deleted_printers==0);
    puts("LeitzLm registry -> OpenPortEx -> owned-port cleanup: PASS");
    return 0;
}
