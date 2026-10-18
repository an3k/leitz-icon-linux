#include "monitor2_native_map.h"
#include "monitor2_port_io.h"
#include "status_dispatch.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t sent[8];
    unsigned sends, reads;
    void *expected_handle;
} port_trace;
static int32_t device_write(void *context,void *handle,const uint8_t *src,
                            uint32_t n,uint32_t *written)
{
    port_trace *t=context;
    assert(handle==t->expected_handle && n==2 && src[0]==0x1b);
    assert(t->sends<3);
    memcpy(t->sent+2*t->sends,src,n);
    ++t->sends;
    *written=n;
    return 1;
}
static int32_t device_read(void *context,void *handle,uint8_t *dst,
                           uint32_t n,uint32_t *received)
{
    port_trace *t=context;
    assert(handle==t->expected_handle && n>=2);
    ++t->reads;
    dst[0]='O';dst[1]='K';
    *received=2;
    return 1;
}
static int32_t exchange(void *ctx,const uint8_t *command,size_t length,
                         uint8_t *answer,size_t capacity,size_t *answer_length)
{
    leitz_win2018_monitor2_port *port=ctx;
    assert(length==2 && capacity>=2);
    uint32_t written=0,read_count=0;
    if (!leitz_win2018_monitor2_write(port,command,(uint32_t)length,&written) ||
        written!=length) return -1;
    if (!leitz_win2018_monitor2_read(port,answer,(uint32_t)capacity,&read_count)) return -1;
    *answer_length=read_count;
    return 0;
}
int main(void)
{
    const leitz_win2018_monitor2_entry *slot=leitz_win2018_monitor2_slot(13);
    assert(slot && slot->role==LEITZ_LM_MONITOR_GET_PRINTER_DATA);
    assert(slot->x86_rva==0x1440 && slot->x64_rva==0x1000);
    int handle=123;port_trace t={.expected_handle=&handle};
    leitz_win2018_monitor2_port port={
        .underlying_handle=&handle,.context=&t,
        .write=device_write,.read=device_read
    };
    static const uint16_t request_a[]={
        'l','e','i','t','z','_','o','e','m','_','s','t','a','t','u','s',0};
    static const uint16_t request_u[]={
        'l','e','i','t','z','_','o','e','m','_','c','a','r','t','r','i','d','g','e',0};
    static const uint16_t request_v[]={
        'l','e','i','t','z','_','o','e','m','_','v','e','r','s','i','o','n',0};
    uint8_t reply[16]={0};size_t n=55;
    assert(leitz_win2018_lm_dispatch(request_a,reply,sizeof reply,&n,exchange,&port)==1);
    assert(n==2 && !memcmp(reply,"OK",2));
    assert(leitz_win2018_lm_dispatch(request_u,reply,sizeof reply,&n,exchange,&port)==1);
    assert(n==2);
    assert(leitz_win2018_lm_dispatch(request_v,reply,sizeof reply,&n,exchange,&port)==1);
    assert(n==2 && t.sends==3 && t.reads==3);
    static const uint8_t expected[]={0x1b,'A',0x1b,'U',0x1b,'V'};
    assert(!memcmp(t.sent,expected,sizeof expected));
    puts("Original MONITOR2 data-from-port status -> ESC A/U/V through delegates: PASS");
    return 0;
}
