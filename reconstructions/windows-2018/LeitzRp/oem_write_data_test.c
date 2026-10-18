#include "oem_write_data.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int calls, reports;
    int32_t result;
    uint32_t written;
    uint32_t last_requested, last_error;
    uint64_t count_on_entry;
    leitz_rp_oem_write_state *s;
    const uint8_t *last_bytes;
    void *device;
} trace;
static int32_t write_bytes(void *ctx, void *device, const uint8_t *data,
                           uint32_t count, uint32_t *written)
{
    trace *t=ctx;
    ++t->calls;
    t->count_on_entry=t->s->accumulated_bytes;
    t->last_bytes=data;
    t->last_requested=count;
    t->device=device;
    *written=t->written;
    return t->result;
}
static int32_t report_error(void *ctx, void *device, const uint8_t *data,
                            uint32_t count, uint32_t code)
{
    trace *t=ctx;
    ++t->reports;
    assert(device==t->device && count==t->last_requested && data==t->last_bytes);
    t->last_error=code;
    return 777; /* Original ignores diagnostic callback's result. */
}
int main(void)
{
    uint8_t data[]={0x1b,'f',0,3};
    trace t={0};int device=17;
    leitz_rp_oem_write_state s={
       .architecture=LEITZ_RP_WRITE_X64,.accumulated_bytes=0,
       .device=&device,.callback_context=&t,.write=write_bytes,.report=report_error
    };
    t.s=&s;t.written=sizeof data;
    assert(leitz_rp_oem_write_data(&s,data,sizeof data)==0);
    assert(t.calls==1&&t.reports==0&&t.count_on_entry==4&&s.accumulated_bytes==4);
    assert(t.device==&device&&t.last_bytes==data);
    t.result=(int32_t)UINT32_C(0x80004002); /* Other failure HRESULT is accepted! */
    assert(leitz_rp_oem_write_data(&s,data,sizeof data)==0);
    assert(t.calls==2&&t.reports==0&&s.accumulated_bytes==8);
    t.result=(int32_t)LEITZ_RP_WRITE_E_FAIL;
    assert((uint32_t)leitz_rp_oem_write_data(&s,data,sizeof data)==LEITZ_RP_WRITE_E_FAIL);
    assert(t.calls==3&&t.reports==1&&t.last_error==200&&s.accumulated_bytes==12);
    t.result=0;t.written=sizeof data-1;
    assert((uint32_t)leitz_rp_oem_write_data(&s,data,sizeof data)==LEITZ_RP_WRITE_E_FAIL);
    assert(t.calls==4&&t.reports==2&&s.accumulated_bytes==16);
    t.written=0;
    assert(leitz_rp_oem_write_callback(&s,NULL,0)==0);
    assert(t.calls==5&&t.reports==2&&s.accumulated_bytes==16);
    t.written=2;
    assert((uint32_t)leitz_rp_oem_write_data(&s,NULL,0)==LEITZ_RP_WRITE_E_FAIL);
    assert(t.calls==6&&t.reports==3&&s.accumulated_bytes==16);
    t.written=sizeof data;
    s.architecture=LEITZ_RP_WRITE_X86;
    s.accumulated_bytes=UINT32_MAX-1u;
    assert(leitz_rp_oem_write_callback(&s,data,sizeof data)==0);
    assert(s.accumulated_bytes==2 && t.count_on_entry==2);
    s.architecture=LEITZ_RP_WRITE_X64;
    s.accumulated_bytes=UINT64_MAX-1u;
    assert(leitz_rp_oem_write_callback(&s,data,sizeof data)==0);
    assert(s.accumulated_bytes==2 && t.count_on_entry==2);
    s.architecture=(leitz_rp_write_arch)16;
    assert((uint32_t)leitz_rp_oem_write_data(&s,data,sizeof data)==LEITZ_RP_WRITE_E_FAIL);
    s.architecture=LEITZ_RP_WRITE_X86;
    const int oldcalls=t.calls;
    assert((uint32_t)leitz_rp_oem_write_data(&s,NULL,sizeof data)==LEITZ_RP_WRITE_E_FAIL);
    assert((uint32_t)leitz_rp_oem_write_data(NULL,data,sizeof data)==LEITZ_RP_WRITE_E_FAIL);
#if SIZE_MAX > UINT32_MAX
    assert((uint32_t)leitz_rp_oem_write_callback(&s,data,(size_t)UINT32_MAX+1u)==LEITZ_RP_WRITE_E_FAIL);
#endif
    assert(t.calls==oldcalls && s.accumulated_bytes==2);
    puts("Windows 2018 original LeitzRp WriteData host callback and byte accounting: PASS");
    return 0;
}
