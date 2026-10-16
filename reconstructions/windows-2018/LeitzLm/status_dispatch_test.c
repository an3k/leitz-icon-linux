#include "status_dispatch.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t commands[8];
    unsigned called;
    int32_t result;
    size_t length;
    const uint8_t *data;
} mock;

static int32_t exchange(void *ctx, const uint8_t *command, size_t size,
                        uint8_t *response, size_t capacity,
                        size_t *received)
{
    mock *m=ctx;
    assert(size==2 && command[0]==0x1b);
    assert(m->called++<4);
    m->commands[(m->called-1)*2]=command[0];
    m->commands[(m->called-1)*2+1]=command[1];
    assert(m->length<=capacity);
    if(m->length)memcpy(response,m->data,m->length);
    *received=m->length;
    return m->result;
}
static void make_wide(uint16_t *out,const char *in)
{
    do {*out++=(uint8_t)*in;}while(*in++);
}
int main(void)
{
    uint16_t query[128]={0};
    uint8_t reply[1024]={0};
    size_t n=888;
    static const uint8_t data[]={'r','e','a','d','y',0};
    mock m={0};
    m.length=sizeof data;m.data=data;

    const char *names[]={
        "leitz_oem_status","leitz_oem_cartridge","leitz_oem_version"
    };
    const uint8_t commands[]={'A','U','V'};
    for(unsigned i=0;i<3;++i){
        make_wide(query,names[i]);
        assert(leitz_win2018_lm_dispatch(query,reply,sizeof reply,
                                          &n,exchange,&m)==1);
        assert(n==sizeof data && !memcmp(reply,data,n));
        assert(m.commands[i*2]==0x1b && m.commands[i*2+1]==commands[i]);
    }
    assert(m.called==3);

    make_wide(query,"leitz_oem_Status");
    n=92;
    assert(leitz_win2018_lm_dispatch(query,reply,sizeof reply,
                                      &n,exchange,&m)==0);
    assert(n==92 && m.called==3);
    make_wide(query,"leitz_oem_cartridgeX");
    assert(leitz_win2018_lm_dispatch(query,reply,sizeof reply,
                                      &n,exchange,&m)==0);
    assert(n==92 && m.called==3);
    assert(leitz_win2018_lm_dispatch(NULL,reply,sizeof reply,
                                      &n,exchange,&m)==-1);
    assert(leitz_win2018_lm_dispatch(query,reply,sizeof reply,
                                      NULL,exchange,&m)==-1);

    make_wide(query,"leitz_oem_status");
    m.result=-1;
    assert(leitz_win2018_lm_dispatch(query,reply,sizeof reply,
                                      &n,exchange,&m)==1);
    assert(n==0 && m.called==4); /* native recognized TRUE */
    assert(leitz_win2018_lm_dispatch(query,NULL,1,
                                      &n,exchange,&m)==-1);

    leitz_win2018_lm_collector c={0};
    const uint8_t first[]={'1','2','3'};
    const uint8_t second[]={'4','5'};
    assert(leitz_win2018_lm_collect(&c,first,3,5)==0);
    assert(c.length==3 && !c.complete);
    assert(leitz_win2018_lm_collect(&c,second,2,5)==1);
    assert(c.length==5 && c.complete);
    assert(!memcmp(c.bytes,"12345",5));
    assert(leitz_win2018_lm_collect(&c,first,3,5)==1 && c.length==5);

    memset(&c,0,sizeof c);
    const uint8_t nul[]={'x','y',0};
    assert(leitz_win2018_lm_collect(&c,nul,sizeof nul,10)==1);
    assert(c.length==3 && c.complete && c.bytes[2]==0);

    memset(&c,0,sizeof c);
    assert(leitz_win2018_lm_collect(&c,NULL,0,0)==0 && c.length==0);
    assert(leitz_win2018_lm_collect(&c,first,3,5)==0);
    assert(leitz_win2018_lm_collect(&c,first,1022,5)==-1);
    assert(c.length==3); /* reject entire overflowing packet */
    assert(leitz_win2018_lm_collect(&c,first,0,1025)==-1);
    assert(leitz_win2018_lm_collect(NULL,first,1,5)==-1);
    assert(leitz_win2018_lm_collect(&c,NULL,1,5)==-1);

    puts("Windows LeitzLm 2018 OEM status dispatch and bounded responses: PASS");
    return 0;
}
