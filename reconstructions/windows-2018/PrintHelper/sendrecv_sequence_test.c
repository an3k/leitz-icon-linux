#include "sendrecv_sequence.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char trace[32];
    size_t used;
    int fail_at;
    int calls;
    int32_t report_status;
    const uint8_t *reply;
    size_t reply_length;
    int freed;
    int released_first, released_second, stopped;
} fixture;

static int hit(fixture *f, char tag)
{
    assert(f->used + 1 < sizeof f->trace);
    f->trace[f->used++]=tag;
    f->trace[f->used]='\0';
    return ++f->calls == f->fail_at ? 1 : 0;
}
static int32_t initialize(void *ctx)
{
    return hit(ctx,'I');
}
static int32_t create(void *ctx, leitz_win2018_com_role role,
                      const char *clsid, const char *iid)
{
    if (role == LEITZ_WIN2018_COM_FIRST) {
        assert(!strcmp(clsid,"2a614240-a4c5-4c33-bd87-1bc709331639"));
        assert(!strcmp(iid,"d580dc0e-de39-4649-baa8-bf0b85a03a97"));
    } else {
        assert(role == LEITZ_WIN2018_COM_SECOND);
        assert(!strcmp(clsid,"b9162a23-45f9-47cc-80f5-fe0fe9b9e1a2"));
        assert(!strcmp(iid,"8f348bd7-4b47-4755-8a9d-0f422df3dc89"));
    }
    return hit(ctx,role==LEITZ_WIN2018_COM_FIRST?'a':'b');
}
static int32_t invoke(void *ctx, leitz_win2018_com_role role,
                      uint32_t slot,const void *argument,size_t bytes,
                      uint32_t literal,int32_t *status,
                      const uint8_t **reply,size_t *reply_length)
{
    fixture *f=ctx;
    char tag='?';
    if(role==LEITZ_WIN2018_COM_FIRST && slot==0x0c) {
        assert(argument==(const void *)f && !bytes && literal==2 &&
               !status && !reply && !reply_length);
        tag='A';
    } else if(role==LEITZ_WIN2018_COM_SECOND && slot==0x0c) {
        static const uint16_t expected[]={
            'R','e','a','d','W','r','i','t','e','U','s','e','r','D','a','t','a',0
        };
        assert(argument && bytes==sizeof expected && !literal &&
               !memcmp(argument,expected,bytes));
        tag='B';
    } else if(role==LEITZ_WIN2018_COM_SECOND && slot==0x10) {
        assert(argument && bytes==3 && literal==7);
        assert(!memcmp(argument,"\x1b\x41\x00",3));
        tag='P';
    } else if(role==LEITZ_WIN2018_COM_FIRST && slot==0x14) {
        static const uint16_t expected[]={'G','e','t',0};
        assert(argument && bytes==sizeof expected && !literal &&
               !memcmp(argument,expected,bytes));
        tag='G';
    } else if(role==LEITZ_WIN2018_COM_SECOND && slot==0x14) {
        assert(status && !argument && !bytes && !literal &&
               !reply && !reply_length);
        *status=f->report_status;
        tag='S';
    } else if(role==LEITZ_WIN2018_COM_SECOND && slot==0x18) {
        assert(!argument && !bytes && !literal && !status &&
               reply && reply_length);
        *reply=f->reply;
        *reply_length=f->reply_length;
        tag='R';
    } else assert(!"unrecovered COM call");
    return hit(f,tag);
}
static void free_reply(void *ctx,const uint8_t *reply)
{
    fixture *f=ctx;
    assert(reply==f->reply);
    ++f->freed;
    (void)hit(f,'F');
}
static void release(void *ctx,leitz_win2018_com_role role)
{
    fixture *f=ctx;
    if(role==LEITZ_WIN2018_COM_SECOND){++f->released_second;(void)hit(f,'y');}
    else{assert(role==LEITZ_WIN2018_COM_FIRST);++f->released_first;(void)hit(f,'x');}
}
static void uninitialize(void *ctx)
{
    fixture *f=ctx;
    ++f->stopped;
    (void)hit(f,'U');
}
static void reset(fixture *f)
{
    static const uint8_t bytes[]={0x1b,'V',0,0xa5};
    memset(f,0,sizeof *f);
    f->reply=bytes;
    f->reply_length=sizeof bytes;
}
static void verify(const fixture *f,const char *expected)
{
    assert(!strcmp(f->trace,expected));
}
int main(void)
{
    fixture f;
    uint8_t out[12];
    const uint8_t payload[]={0x1b,'A',0};
    leitz_win2018_sendrecv_com_ops ops={
        &f,initialize,create,invoke,free_reply,release,uninitialize
    };
    size_t written=91;
    leitz_win2018_sendrecv_request request={
        &f,payload,sizeof payload,out,sizeof out,&written
    };
    reset(&f);memset(out,0xcc,sizeof out);
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==1);
    verify(&f,"IaAbBPGSRFyxU");
    assert(written==4 && !memcmp(out,f.reply,written));
    assert(out[4]==0xcc && f.freed==1 && f.released_first==1 &&
           f.released_second==1 && f.stopped==1);

    /* No reply: false without publishing output or freeing a NULL source. */
    reset(&f);f.reply=NULL;f.reply_length=0;written=91;
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==0);
    verify(&f,"IaAbBPGSRyxU");assert(written==91 && f.freed==0);

    /* Native SECOND+0x14 requires both HRESULT and returned status zero. */
    reset(&f);f.report_status=1;written=91;
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==0);
    verify(&f,"IaAbBPGSyxU");assert(written==91 && !f.freed);

    /* Early-stage failures stop safely; only acquired objects released. */
    static const char *paths[]={"I","IaU","IaAxU",
                                  "IaAbxU","IaAbByxU",
                                  "IaAbBPyxU","IaAbBPGyxU",
                                  "IaAbBPGSyxU","IaAbBPGSRFyxU"};
    /* Failure positions 1..9 (before cleanup events are counted). */
    for(int fail=1;fail<=9;++fail) {
        reset(&f);f.fail_at=fail;written=91;
        assert(leitz_win2018_sendrecv_sequence(&ops,&request)==0);
        verify(&f,paths[fail-1]);
        assert(written==91);
    }

    /* Bounded response refuses overlong COM result and always frees it. */
    reset(&f);written=91;request.response_capacity=2;
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==-1);
    verify(&f,"IaAbBPGSRFyxU");
    assert(written==91 && out[4]==0xcc && f.freed==1);
    request.response_capacity=sizeof out;

    reset(&f);written=91;request.original_first_argument=NULL;
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==-1);
    assert(!f.used && written==91);
    request.original_first_argument=&f;
    reset(&f);written=91;ops.release=NULL;
    assert(leitz_win2018_sendrecv_sequence(&ops,&request)==-1);
    assert(!f.used && written==91);

    puts("Windows PrintHelper 2018 COM SendRecvData callback sequence: PASS");
    return 0;
}
