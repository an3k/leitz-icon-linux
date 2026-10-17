#include "oem_row_dispatch.h"
#include "label_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint8_t bytes[128]; size_t len, calls; } sink;
static int32_t collect(void *ctx,const uint8_t *data,size_t len) {
    sink *s=ctx;
    assert(s->len+len<=sizeof s->bytes);
    memcpy(s->bytes+s->len,data,len);
    s->len+=len;
    s->calls++;
    return 0;
}
typedef struct {size_t calls;uint16_t last_row;} alternate_trace;
static int32_t continuous_unrecovered(void *ctx,leitz_win2018_raster_stream *stream,
                                      const uint8_t *pixels,uint32_t len) {
    alternate_trace *t=ctx;
    assert(!len || pixels);
    t->last_row=stream->row->row_index;
    t->calls++;
    return -23;
}
int main(void) {
    leitz_win2018_row_control row={0};
    leitz_win2018_label_result info={0};
    leitz_win2018_label_input input={"CUSTOMSIZE",16,3,1,0,0};
    uint8_t clipped[8], encoded[8];
    sink output={0};
    leitz_win2018_raster_stream stream={
        &row,collect,&output,clipped,encoded,sizeof clipped,0
    };
    const uint8_t white[]={0,0};
    assert(leitz_win2018_initialize_label(&input,&info,&row)==0);
    assert(info.is_die_cut==1 && row.label_height==3);
    for(unsigned k=1;k<=3;++k) {
        assert(leitz_win2018_oem_dispatch_row(&stream,white,sizeof white,
                                              info.is_die_cut,NULL,NULL)==0);
        if(k<3) assert(row.row_index==k);
    }
    const uint8_t expected[]={0x1b,'w',0,16, 0x1b,'f',0,3,0x1b,'G'};
    assert(output.len==sizeof expected && !memcmp(output.bytes,expected,sizeof expected));
    assert(output.calls==3 && row.row_index==0 && row.label_count==2);
    /* Continuous OEM medium dispatch goes into a DIFFERENT original
       routine, not into the (already reconstructed) die-cut path. */
    memset(&output,0,sizeof output);
    input.paper_name="Continuous70030001";input.raster_height=2;
    input.configured_length=4;
    assert(leitz_win2018_initialize_label(&input,&info,&row)==0);
    assert(info.is_die_cut==0 && row.label_height==4);
    assert(leitz_win2018_oem_dispatch_row(&stream,white,sizeof white,
                                          info.is_die_cut,NULL,NULL)==
           LEITZ_WIN2018_OEM_ALT_PATH_MISSING);
    assert(row.row_index==0 && output.len==0);
    alternate_trace t={0};
    assert(leitz_win2018_oem_dispatch_row(&stream,white,sizeof white,
                                        info.is_die_cut,continuous_unrecovered,&t)==-23);
    assert(row.row_index==1 && t.calls==1 && t.last_row==1 && output.len==0);
    puts("Original LeitzRp IsDieCut dispatcher + existing real raster stream: PASS");
    return 0;
}
