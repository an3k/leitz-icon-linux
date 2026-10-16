#include "row_control.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[512];
    size_t len, calls, encode_calls;
    uint32_t encode_total, encode_leading;
    size_t fail_call;
    int32_t failed_result, encode_result;
    leitz_win2018_row_control *state;
    uint16_t label_on_g, width_on_g, pending_on_g;
} fixture;

static int32_t writer(void *ctx, const uint8_t *b, size_t n)
{
    fixture *f = ctx;
    assert(f->len+n <= sizeof f->bytes);
    ++f->calls;
    if (n==2 && b[0]==0x1b && b[1]=='G') {
        f->label_on_g=f->state->label_count;
        f->width_on_g=f->state->previous_trimmed_width;
        f->pending_on_g=f->state->queued_blank_rows;
    }
    /* Keep attempted bytes even when the mock reports E_FAIL. */
    memcpy(f->bytes+f->len,b,n); f->len+=n;
    if (f->calls==f->fail_call)
        return f->failed_result;
    return 0;
}
static int32_t encoder(void *ctx, const uint8_t *r, uint32_t n, uint32_t leading)
{
    fixture *f=ctx;
    assert(r && leading<n && r[leading]!=0);
    ++f->encode_calls;
    f->encode_total=n;
    f->encode_leading=leading;
    return f->encode_result;
}
static void begin(fixture *f, leitz_win2018_row_control *s)
{
    memset(f,0,sizeof *f);
    f->state=s;
}
static void check(const fixture *f, const uint8_t *bytes, size_t n)
{
    assert(f->len==n && !memcmp(f->bytes,bytes,n));
}
int main(void)
{
    fixture f;
    leitz_win2018_row_control s={0};
    const uint8_t white[5]={0,0,0,0,0};
    const uint8_t nonblank[4]={0,0,0x55,0};

    s.label_count=1; s.label_height=5; s.row_index=2;
    begin(&f,&s);
    assert(leitz_win2018_process_counted_row(&s,white,5,writer,encoder,&f)==0);
    assert(!f.calls && !f.encode_calls && s.queued_blank_rows==1);
    s.row_index=3;
    assert(leitz_win2018_process_counted_row(&s,NULL,0,writer,encoder,&f)==0);
    assert(!f.calls && s.queued_blank_rows==2);
    s.row_index=4;
    assert(leitz_win2018_process_counted_row(&s,nonblank,4,writer,encoder,&f)==0);
    const uint8_t feed[]={0x1b,'f',0,2};
    check(&f,feed,sizeof feed);
    assert(f.calls==1 && f.encode_calls==1 && f.encode_total==4 && f.encode_leading==2);
    assert(s.queued_blank_rows==0 && s.label_count==1);

    s.row_index=5;s.previous_trimmed_width=24;
    begin(&f,&s);
    assert(leitz_win2018_process_counted_row(&s,white,5,writer,encoder,&f)==0);
    const uint8_t endblank[]={0x1b,'f',0,1,0x1b,'G'};
    check(&f,endblank,sizeof endblank);
    assert(s.label_count==2 && s.row_index==0 && s.queued_blank_rows==0);
    assert(s.previous_trimmed_width==0xffff && f.width_on_g==0xffff && f.pending_on_g==0);
    assert(f.label_on_g==1 && f.encode_calls==0);

    /* First counted row: width header, medium command, raster callback. */
    memset(&s,0,sizeof s); s.row_index=1; s.label_height=5;
    s.send_width_flag=1; s.physical_width=0x1234; s.medium_switch_flag=1;
    begin(&f,&s);
    assert(leitz_win2018_process_counted_row(&s,nonblank,4,writer,encoder,&f)==0);
    const uint8_t setup[]={0x1b,'w',0x12,0x34,0x1b,'E'};
    check(&f,setup,sizeof setup);
    assert(f.calls==2 && f.encode_calls==1);

    s.row_index=2; s.send_width_flag=0; s.medium_switch_flag=0;
    s.queued_blank_rows=4;
    begin(&f,&s); f.fail_call=1; f.failed_result=LEITZ_WIN2018_ROW_E_FAIL;
    assert(leitz_win2018_process_counted_row(&s,nonblank,4,writer,encoder,&f)
           == LEITZ_WIN2018_ROW_E_FAIL);
    assert(f.calls==1 && f.encode_calls==0 && s.queued_blank_rows==0);

    s.queued_blank_rows=7;
    begin(&f,&s); f.fail_call=1; f.failed_result=(int32_t)0x80070005u;
    assert(leitz_win2018_process_counted_row(&s,nonblank,4,writer,encoder,&f)==0);
    assert(f.encode_calls==1 && s.queued_blank_rows==0);

    /* The OEM ignores the compressor's HRESULT and the G-send result. */
    s.row_index=s.label_height; s.previous_trimmed_width=3;
    begin(&f,&s); f.encode_result=LEITZ_WIN2018_ROW_E_FAIL;
    f.fail_call=1; f.failed_result=LEITZ_WIN2018_ROW_E_FAIL;
    assert(leitz_win2018_process_counted_row(&s,nonblank,4,writer,encoder,&f)==0);
    const uint8_t endlabel[]={0x1b,'G'};
    check(&f,endlabel,sizeof endlabel);
    assert(f.calls==1 && f.encode_calls==1);
    assert(s.label_count==1 && s.row_index==0 && s.previous_trimmed_width==0xffff);
    assert(leitz_win2018_process_counted_row(&s,NULL,1,writer,encoder,&f)
           == LEITZ_WIN2018_ROW_E_FAIL);

    puts("Windows 2018 LeitzRp row control ESC w/E/f/G: PASS");
    return 0;
}
