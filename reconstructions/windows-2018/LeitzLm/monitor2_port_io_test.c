#include "monitor2_port_io.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    int write_calls, read_calls, errors;
    int32_t write_result,read_result;
    uint32_t written, read_count, code, last_requested;
    void *expected;
    uint8_t last_bytes[8];
} trace;
static int32_t write_it(void *ctx,void *underlying,const uint8_t *data,
                         uint32_t count,uint32_t *transferred)
{
    trace *t=ctx;
    assert(t->expected==underlying);
    assert(!count || data);
    assert(count<=sizeof t->last_bytes);
    if(count)memcpy(t->last_bytes,data,count);
    ++t->write_calls;t->last_requested=count;
    *transferred=t->written;
    return t->write_result;
}
static int32_t read_it(void *ctx,void *underlying,uint8_t *data,
                        uint32_t count,uint32_t *transferred)
{
    trace *t=ctx;
    assert(t->expected==underlying);
    assert(!count || data);
    if(count)data[0]=0x1b;
    ++t->read_calls;t->last_requested=count;
    *transferred=t->read_count;
    return t->read_result;
}
static void error(void *ctx,uint32_t code) {
    trace *t=ctx;++t->errors;t->code=code;
}
int main(void) {
    int port_handle=17;
    trace t={.expected=&port_handle,.written=2,.read_count=1,
             .write_result=71,.read_result=-55};
    leitz_win2018_monitor2_port port={
        .underlying_handle=&port_handle,.context=&t,
        .write=write_it,.read=read_it,.error=error
    };
    uint8_t data[]={0x1b,'G',0};uint32_t count=999;
    assert(leitz_win2018_monitor2_write(&port,data,3,&count)==71);
    assert(count==2&&t.write_calls==1&&t.last_requested==3);
    assert(t.last_bytes[0]==0x1b&&t.last_bytes[1]=='G'&&t.errors==0);
    count=999;
    assert(leitz_win2018_monitor2_read(&port,data,3,&count)==-55);
    assert(t.read_calls==1&&count==1&&data[0]==0x1b&&t.errors==0);
    t.write_result=0;t.written=0;
    assert(leitz_win2018_monitor2_write(&port,NULL,0,&count)==0);
    assert(t.write_calls==2&&count==0);
    t.read_result=1;t.read_count=0;
    assert(leitz_win2018_monitor2_read(&port,NULL,0,&count)==1);
    assert(t.read_calls==2&&count==0);
    count=777;
    assert(leitz_win2018_monitor2_write(&port,NULL,1,&count)==0);
    assert(t.errors==1&&t.code==87&&t.write_calls==2&&count==777);
    assert(leitz_win2018_monitor2_write(&port,data,3,NULL)==0);
    assert(t.errors==2&&t.write_calls==2);
    assert(leitz_win2018_monitor2_read(&port,NULL,1,&count)==0);
    assert(leitz_win2018_monitor2_read(&port,data,3,NULL)==0);
    assert(t.read_calls==2&&t.errors==2);
    port.write=NULL;
    assert(leitz_win2018_monitor2_write(&port,data,3,&count)==0);
    assert(t.errors==3);
    port.read=NULL;
    assert(leitz_win2018_monitor2_read(&port,data,3,&count)==0);
    assert(t.errors==3);
    assert(leitz_win2018_monitor2_write(NULL,data,3,&count)==0);
    assert(leitz_win2018_monitor2_read(NULL,data,3,&count)==0);
    assert(t.write_calls==2&&t.read_calls==2);
    puts("Original LeitzLm MONITOR2 WritePort/ReadPort host forwarding: PASS");
    return 0;
}
