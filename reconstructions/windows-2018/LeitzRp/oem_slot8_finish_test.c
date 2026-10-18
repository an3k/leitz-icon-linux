#include "oem_slot8_finish.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    int writes,reports,free_calls;
    uint8_t byte;
    uint32_t length,code;
    uint32_t returned_written;
    int32_t returned_status;
    void *freed[2];
} trace;
static int32_t write_spool(void *c,void *device,const uint8_t *bytes,
                          uint32_t size,uint32_t *written)
{
    trace *t=c;(void)device;
    ++t->writes;
    assert(bytes && size==1);
    t->byte=bytes[0]; t->length=size;
    *written=t->returned_written;
    return t->returned_status;
}
static int32_t report(void *c,void *device,const uint8_t *bytes,
                      uint32_t size,uint32_t code)
{
    trace *t=c;(void)device;
    assert(bytes && size==1);
    ++t->reports; t->code=code;
    return (int32_t)LEITZ_RP_WRITE_E_FAIL;
}
static void free_res(void *c,void *resource)
{
    trace *t=c;
    assert(t->free_calls<2);
    t->freed[t->free_calls++]=resource;
}
int main(void)
{
    trace t={0};int block=3, nested=7;
    leitz_rp_oem_write_state w={.architecture=LEITZ_RP_WRITE_X86,
        .accumulated_bytes=26,.callback_context=&t,
        .write=write_spool,.report=report};
    leitz_rp_oem_slot8_arguments a={.medium_word_at_0c=2,
        .allocated_block=&block,.nested_allocated_resource=&nested,
        .free_resource=free_res,.free_context=&t};
    t.returned_written=1;
    assert(leitz_rp_oem_slot8_finish(&w,&a)==0);
    assert(t.writes==1&&t.byte==0x1b&&t.length==1&&t.reports==0);
    assert(w.accumulated_bytes==27);
    assert(t.free_calls==2&&t.freed[0]==&nested&&t.freed[1]==&block);
    /* No ESC when count is not exactly 5*medium WORD+16. */
    t.free_calls=0;t.writes=0;t.reports=0;
    w.accumulated_bytes=25;
    assert(leitz_rp_oem_slot8_finish(&w,&a)==0);
    assert(t.writes==0&&w.accumulated_bytes==25&&t.free_calls==2);
    /* Original return remains S_OK even when WriteData reports E_FAIL. */
    t.free_calls=0;t.writes=0;t.reports=0;
    w.accumulated_bytes=26;t.returned_written=0;
    t.returned_status=(int32_t)LEITZ_RP_WRITE_E_FAIL;
    assert(leitz_rp_oem_slot8_finish(&w,&a)==0);
    assert(t.writes==1&&t.reports==1&&t.code==200&&t.free_calls==2);
    assert(w.accumulated_bytes==27);
    /* x86 32-bit accumulator comparison versus x64 full 64-bit. */
    t.free_calls=0;t.writes=0;t.reports=0;
    t.returned_written=1;t.returned_status=0;
    w.accumulated_bytes=((uint64_t)1<<32)+26u;
    assert(leitz_rp_oem_slot8_finish(&w,&a)==0);
    assert(t.writes==1&&w.accumulated_bytes==27u);
    t.free_calls=0;t.writes=0;t.reports=0;
    w.architecture=LEITZ_RP_WRITE_X64;
    w.accumulated_bytes=((uint64_t)1<<32)+26u;
    assert(leitz_rp_oem_slot8_finish(&w,&a)==0);
    assert(t.writes==0&&w.accumulated_bytes==((uint64_t)1<<32)+26u);
    /* Invalid buffers rejected before any cleanup or spool side effects. */
    t.free_calls=0;t.writes=0;
    assert((uint32_t)leitz_rp_oem_slot8_finish(NULL,&a)==LEITZ_RP_WRITE_E_FAIL);
    assert((uint32_t)leitz_rp_oem_slot8_finish(&w,NULL)==LEITZ_RP_WRITE_E_FAIL);
    a.free_resource=NULL;
    assert((uint32_t)leitz_rp_oem_slot8_finish(&w,&a)==LEITZ_RP_WRITE_E_FAIL);
    assert(t.free_calls==0&&t.writes==0);
    puts("Original LeitzRp raster slot 8 one-byte ESC and cleanup: PASS");
    return 0;
}
