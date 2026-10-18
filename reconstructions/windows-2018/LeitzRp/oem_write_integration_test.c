#include "oem_write_data.h"
#include "raster_stream.h"
#include "label_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[256];
    uint32_t lengths[32];
    unsigned calls, reports;
    size_t size;
    unsigned short_write_call;
    uint64_t count_at_call[32];
    leitz_rp_oem_write_state *writer;
} spool_trace;

static int32_t write_spool(void *ctx, void *device,
    const uint8_t *bytes, uint32_t requested, uint32_t *written)
{
    spool_trace *t=ctx;
    (void)device;
    assert(t->calls < 32);
    assert(t->size + requested <= sizeof t->bytes);
    t->lengths[t->calls]=requested;
    t->count_at_call[t->calls]=t->writer->accumulated_bytes;
    ++t->calls;
    if (requested) memcpy(t->bytes+t->size, bytes, requested);
    t->size+=requested;
    *written=t->calls==t->short_write_call ? requested-1u : requested;
    return 0;
}
static int32_t report(void *ctx, void *device, const uint8_t *bytes,
    uint32_t requested, uint32_t error_code)
{
    spool_trace *t=ctx;
    (void)device;(void)bytes;(void)requested;
    assert(error_code==200);
    ++t->reports;
    return 10;
}
int main(void)
{
    spool_trace sink={0};
    leitz_rp_oem_write_state writer={
       .architecture=LEITZ_RP_WRITE_X64,
       .accumulated_bytes=0,
       .callback_context=&sink,
       .write=write_spool,
       .report=report
    };
    sink.writer=&writer;
    uint8_t clip[8]={0},compressed[8]={0};
    leitz_win2018_row_control row={0};
    leitz_win2018_label_input label={"CUSTOMSIZE",16,1,1,0,1};
    leitz_win2018_label_result result={0};
    leitz_win2018_raster_stream s={
        .row=&row,.write=leitz_rp_oem_write_callback,
        .write_context=&writer,.clipped_row=clip,
        .encoded_row=compressed,.scratch_capacity=sizeof clip
    };
    assert(leitz_win2018_initialize_label(&label,&result,&row)==0);
    row.row_index=1;
    const uint8_t white[]={0,0};
    assert(leitz_win2018_stream_counted_row(&s,white,sizeof white)==0);
    const uint8_t expected[]={0x1b,'w',0,16,0x1b,'f',0,1,0x1b,'G'};
    assert(sink.size==sizeof expected);
    assert(memcmp(sink.bytes,expected,sizeof expected)==0);
    assert(sink.calls==3 && sink.reports==0);
    assert(sink.lengths[0]==4 && sink.lengths[1]==4 && sink.lengths[2]==2);
    assert(sink.count_at_call[0]==4 && sink.count_at_call[1]==8 && sink.count_at_call[2]==10);
    assert(writer.accumulated_bytes==10);
    assert(row.label_count==2 && row.row_index==0);
    /* Original row controller ignores the HRESULT of the final blank
       run write, even though the OEM helper calls the error reporter. */
    memset(&sink,0,sizeof sink);
    sink.writer=&writer;
    writer.accumulated_bytes=0;
    sink.short_write_call=2;
    assert(leitz_win2018_initialize_label(&label,&result,&row)==0);
    row.row_index=1;
    assert(leitz_win2018_stream_counted_row(&s,white,sizeof white)==0);
    assert(sink.calls==3 && sink.reports==1);
    assert(writer.accumulated_bytes==10 && row.label_count==2);
    puts("Windows LeitzRp original WriteData + existing die-cut raster stream: PASS");
    return 0;
}
