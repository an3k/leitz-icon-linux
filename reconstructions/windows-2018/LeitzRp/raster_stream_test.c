#include "raster_stream.h"
#include "label_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[256];
    size_t size;
    size_t calls;
    size_t lengths[32];
    size_t fail_call;
    int32_t fail_value;
} sink;

static int32_t collect(void *context, const uint8_t *data, size_t length)
{
    sink *s=context;
    assert(s->calls < 32 && s->size + length <= sizeof s->bytes);
    s->lengths[s->calls++] = length;
    if (length) memcpy(s->bytes + s->size, data, length);
    s->size += length;
    return s->calls == s->fail_call ? s->fail_value : 0;
}
static void check(const sink *s, const uint8_t *bytes,
                  size_t length, size_t calls)
{
    assert(s->size == length && s->calls == calls);
    if(memcmp(s->bytes, bytes, length)) { fputs("actual: ",stderr);for(size_t i=0;i<length;++i)fprintf(stderr,"%02x ",s->bytes[i]);fputs("\nwant:   ",stderr);for(size_t i=0;i<length;++i)fprintf(stderr,"%02x ",bytes[i]);fputs("\n",stderr); assert(0); }
}

int main(void)
{
    leitz_win2018_row_control row={0};
    leitz_win2018_label_result setup={0};
    leitz_win2018_label_input input={"CUSTOMSIZE",16,3,1,0,170};
    sink out={0};
    uint8_t clip[4]={0}, encoded[4]={0};
    leitz_win2018_raster_stream stream={
        &row,collect,&out,clip,encoded,sizeof clip,0
    };
    const uint8_t white[]={0,0};
    const uint8_t a[]={0,0xff};
    const uint8_t b[]={0xff,0xff};
    assert(leitz_win2018_initialize_label(&input,&setup,&row)==0);
    assert(setup.label_length==3 && row.previous_trimmed_width==UINT16_MAX);
    row.medium_switch_flag=1;

    row.row_index=1;
    assert(leitz_win2018_stream_counted_row(&stream,white,2)==0);
    row.row_index=2;
    assert(leitz_win2018_stream_counted_row(&stream,a,2)==0);
    row.row_index=3;
    assert(leitz_win2018_stream_counted_row(&stream,b,2)==0);
    assert(row.label_count==2 && row.row_index==0 &&
           row.previous_trimmed_width==UINT16_MAX);
    row.row_index=1;
    assert(leitz_win2018_stream_counted_row(&stream,b,2)==0);
    row.row_index=2;
    assert(leitz_win2018_stream_counted_row(&stream,white,2)==0);
    row.row_index=3;
    assert(leitz_win2018_stream_counted_row(&stream,white,2)==0);
    const uint8_t expected[]={
        0x1b,'w',0,16,
        0x1b,'f',0,1, 0x1b,'D',1, 0x1b,'B',1, 0x16,0xff,
        0x1b,'D',2, 0x1b,'B',0, 0x17,0x8f, 0x1b,'G',
        0x1b,'w',0,16, 0x1b,'E', 0x1b,'D',2, 0x1b,'B',0,0x17,0x8f,
        0x1b,'f',0,2, 0x1b,'G'
    };
    check(&out,expected,sizeof expected,19);
    const size_t expected_lengths[]={
        4,4,3,3,1,1,3,3,1,1,2,4,2,3,3,1,1,4,2
    };
    /* 19 separate OEM-shaped WriteData calls, not one concatenated write. */
    assert(out.calls == sizeof expected_lengths/sizeof expected_lengths[0]);
    for(size_t i=0;i<out.calls;++i)
        assert(out.lengths[i]==expected_lengths[i]);
    assert(row.label_count==3 && row.row_index==0);

    /* A fresh label with continuous manual length must not be capped by
     * the raster input height (and no global 170mm cap is introduced). */
    memset(&out,0,sizeof out);
    input.paper_name="Continuous70030001";
    input.raster_height=2;
    input.configured_length=4;
    assert(leitz_win2018_initialize_label(&input,&setup,&row)==0);
    for(uint16_t i=1;i<=4;++i) {
        row.row_index=i;
        assert(leitz_win2018_stream_counted_row(&stream,white,2)==0);
    }
    const uint8_t continuous[]={0x1b,'f',0,4,0x1b,'G'};
    check(&out,continuous,sizeof continuous,2);

    /* Final unused bits masked in the scratch copy, not caller memory. */
    memset(&out,0,sizeof out);
    input.paper_name="CUSTOMSIZE";input.raster_width=9;input.raster_height=1;
    assert(leitz_win2018_initialize_label(&input,&setup,&row)==0);
    const uint8_t unmasked[]={0xff,0xff};
    row.row_index=1;
    assert(leitz_win2018_stream_counted_row(&stream,unmasked,2)==0);
    assert(unmasked[1]==0xff && clip[1]==0x80);
    const uint8_t clipped[]={0x1b,'w',0,9,0x1b,'D',2,0x1b,'B',0,
                             0x16,0xff,0x80,0x1b,'G'};
    check(&out,clipped,sizeof clipped,6);

    /* No state mutation on invalid caller-owned buffers. */
    const uint16_t old_count=row.label_count;
    row.row_index=1;
    stream.scratch_capacity=1;
    assert(leitz_win2018_stream_counted_row(&stream,unmasked,2)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.row_index==1 && row.label_count==old_count);
    stream.scratch_capacity=sizeof clip;
    stream.encoded_row=clip;
    assert(leitz_win2018_stream_counted_row(&stream,unmasked,2)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.row_index==1 && row.label_count==old_count);
    stream.encoded_row=encoded;

    /* Encoder errors are visible diagnostically but the recovered OEM
     * row controller discards their HRESULT. */
    memset(&out,0,sizeof out);
    row.row_index=1;
    out.fail_call=3;
    out.fail_value=LEITZ_WIN2018_ROW_E_FAIL; /* ESC D fails */
    assert(leitz_win2018_stream_counted_row(&stream,unmasked,2)==0);
    assert(stream.last_filter_result==LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.previous_trimmed_width==UINT16_MAX);
    assert(out.calls==4); /* ESC w, ESC E, failed ESC D, ESC G */
    puts("Windows LeitzRp full counted-row raster stream: PASS");
    return 0;
}
