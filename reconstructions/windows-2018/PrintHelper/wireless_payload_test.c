#include "wireless_payload.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned converts,writes;
    int fail_conversion,fail_write,omit_terminator,embedded_zero;
    size_t force_report,supplied_units,delivered_length;
    uint8_t delivered[64];
} fixture;
static int to_ansi(void *ctx,const uint16_t *s,size_t n,
                   uint8_t *buf,size_t cap,size_t *produced)
{
    fixture *f=ctx;
    ++f->converts;f->supplied_units=n;
    if(f->fail_conversion)return 0;
    if(n>cap)return 0;
    for(size_t i=0;i<n;++i)
        /* Deliberately synthetic converter; NOT a CP_ACP guarantee. */
        buf[i]=(uint8_t)(s[i]==0x00e4?0xe4:s[i]);
    if(f->embedded_zero && n>2)buf[1]=0;
    if(f->omit_terminator)buf[n-1]=0x58;
    *produced=f->force_report?f->force_report:n;
    return 1;
}
static int write_data(void *ctx,const uint8_t *data,size_t n)
{
    fixture *f=ctx;
    ++f->writes;
    assert(n<=sizeof f->delivered);
    f->delivered_length=n;
    if(n)memcpy(f->delivered,data,n);
    return !f->fail_write;
}
static void check_wiped(const uint8_t *buf,size_t n)
{
    for(size_t i=0;i<n;++i)assert(buf[i]==0);
}
int main(void)
{
    uint8_t scratch[64];
    fixture f={0};
    const uint16_t ascii[]={'S','S','I','D',':','x',0};
    memset(scratch,0x55,sizeof scratch);
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==1);
    assert(f.converts==1 && f.writes==1 && f.supplied_units==7 &&
           f.delivered_length==6 && !memcmp(f.delivered,"SSID:x",6));
    check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);
    const uint16_t nonascii[]={'X',0x00e4,0};
    assert(leitz_win2018_prepare_wireless_send(nonascii,3,scratch,
        sizeof scratch,to_ansi,write_data,&f)==1);
    assert(f.delivered_length==2 && f.delivered[0]=='X' && f.delivered[1]==0xe4);
    check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);f.embedded_zero=1;
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==1);
    assert(f.delivered_length==1 && f.delivered[0]=='S');
    check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);f.fail_write=1;
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==0);
    assert(f.writes==1);check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);f.fail_conversion=1;
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==0);
    assert(f.writes==0);check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);f.omit_terminator=1;
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==0);
    assert(f.writes==0);check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);f.force_report=100;
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,write_data,&f)==-1);
    assert(f.writes==0);check_wiped(scratch,sizeof scratch);

    memset(&f,0,sizeof f);
    const uint16_t missing_nul[]={'A','B','C'};
    assert(leitz_win2018_prepare_wireless_send(missing_nul,3,scratch,
        sizeof scratch,to_ansi,write_data,&f)==-1);
    assert(f.converts==0 && f.writes==0);
    assert(leitz_win2018_prepare_wireless_send(NULL,3,scratch,
        sizeof scratch,to_ansi,write_data,&f)==-1);
    assert(leitz_win2018_prepare_wireless_send(ascii,7,NULL,
        sizeof scratch,to_ansi,write_data,&f)==-1);
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,0,
        to_ansi,write_data,&f)==-1);
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,NULL,write_data,&f)==-1);
    assert(leitz_win2018_prepare_wireless_send(ascii,7,scratch,
        sizeof scratch,to_ansi,NULL,&f)==-1);

    memset(&f,0,sizeof f);
    const uint16_t empty[]={0};
    assert(leitz_win2018_prepare_wireless_send(empty,1,scratch,
        sizeof scratch,to_ansi,write_data,&f)==1);
    assert(f.writes==1 && f.delivered_length==0);
    check_wiped(scratch,sizeof scratch);
    puts("Windows PrintHelper SetPrinterWirelessNetwork payload conversion: PASS");
    return 0;
}
