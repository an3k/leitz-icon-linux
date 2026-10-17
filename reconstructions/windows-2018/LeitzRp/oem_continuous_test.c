#include "oem_continuous.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[96];
    size_t used, calls, encodes;
    uint32_t last_leading;
    size_t fail_call;
    int32_t send_status, encode_status;
} trace;
static int32_t write_bytes(void *ctx,const uint8_t *buf,size_t n)
{
    trace *t=ctx;
    assert(t->used+n<=sizeof t->bytes);
    memcpy(t->bytes+t->used,buf,n);
    t->used+=n;
    ++t->calls;
    return t->calls==t->fail_call?t->send_status:0;
}
static int32_t encode(void *ctx,const uint8_t *data,uint32_t n,uint32_t leading)
{
    trace *t=ctx;
    assert(data && leading<n && data[leading]!=0);
    ++t->encodes;t->last_leading=leading;
    return t->encode_status;
}
int main(void)
{
    const uint8_t white[]={0,0},ink[]={0,0xff};
    leitz_win2018_row_control row={0};
    leitz_win2018_continuous_state s={&row,100,2,0,0};
    trace t={0};
    row.label_count=1;row.label_height=20;
    for(unsigned i=1;i<=14;i++){
        row.row_index=(uint16_t)i;
        assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==0);
    }
    assert(t.used==0 && t.encodes==0);
    row.row_index=15;
    assert(leitz_win2018_process_continuous_row(&s,white,2,write_bytes,encode,&t)==0);
    assert(row.queued_blank_rows==1);
    row.row_index=16;t.encode_status=23;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==23);
    const uint8_t flushed[]={0x1b,'f',0,1};
    assert(t.used==4 && memcmp(t.bytes,flushed,4)==0 && t.encodes==1);
    assert(row.queued_blank_rows==0 && t.last_leading==1);
    row.row_index=20;
    assert(leitz_win2018_process_continuous_row(&s,white,2,write_bytes,encode,&t)==0);
    assert(row.queued_blank_rows==0 && t.used==8);
    row.row_index=21;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==0);
    const uint8_t ended[]={0x1b,'f',0,1,0x1b,'f',0,1,0x1b,'G'};
    assert(row.label_count==2 && row.previous_trimmed_width==UINT16_MAX);
    assert(t.used==sizeof ended && !memcmp(t.bytes,ended,sizeof ended));
    row.row_index=1;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==0);
    assert(t.used==sizeof ended+2 && t.bytes[sizeof ended]==0x1b && t.bytes[sizeof ended+1]=='E');
    /* Original ESC E has no dependence on medium_switch_flag. */
    t.fail_call=0; t.calls=0;t.used=0;t.encodes=0;
    s.is_custom_size=1;row.label_count=1;row.label_height=50;
    row.row_index=2;row.queued_blank_rows=3;
    t.fail_call=1;t.send_status=LEITZ_WIN2018_ROW_E_FAIL;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.queued_blank_rows==0 && t.encodes==0);
    t.calls=0;t.used=0;row.queued_blank_rows=2;
    t.send_status=-77;t.encode_status=-32;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==-32);
    assert(t.encodes==1 && row.queued_blank_rows==0);
    /* Blank row at height flushes manually but not in automatic mode. */
    t.calls=0;t.used=0;t.fail_call=1;t.send_status=LEITZ_WIN2018_ROW_E_FAIL;
    row.row_index=50;row.queued_blank_rows=3;
    assert(leitz_win2018_process_continuous_row(&s,white,2,write_bytes,encode,&t)==0);
    assert(row.queued_blank_rows==0 && t.used==4);
    s.mode_auto=1;t.used=0;row.queued_blank_rows=3;
    assert(leitz_win2018_process_continuous_row(&s,white,2,write_bytes,encode,&t)==0);
    assert(row.queued_blank_rows==4 && t.used==0);
    /* Paper-height reset precedes row processing. */
    s.mode_auto=0;s.paper_height=12;row.row_index=12;
    row.previous_trimmed_width=7;row.queued_blank_rows=9;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==-32);
    assert(row.row_index==0 && row.previous_trimmed_width==UINT16_MAX);
    s.label_limit=0;row.label_count=1;row.row_index=3;
    const size_t prior=t.used;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==0);
    assert(t.used==prior);
    row.label_height=0;
    assert(leitz_win2018_process_continuous_row(&s,ink,2,write_bytes,encode,&t)==LEITZ_WIN2018_ROW_E_FAIL);
    assert(leitz_win2018_process_continuous_row(NULL,ink,2,write_bytes,encode,&t)==LEITZ_WIN2018_ROW_E_FAIL);
    assert(leitz_win2018_process_continuous_row(&s,NULL,2,write_bytes,encode,&t)==LEITZ_WIN2018_ROW_E_FAIL);
    puts("Windows OEM continuous-media state, length and cut transitions: PASS");
    return 0;
}
