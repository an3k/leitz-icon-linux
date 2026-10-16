#include "label_setup.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[128];
    size_t length, calls, encoded;
    leitz_win2018_row_control *row;
    uint16_t labels_seen_at_end;
} io_fixture;

static int32_t capture(void *ctx,const uint8_t *data,size_t n)
{
    io_fixture *f=ctx;
    assert(f->length+n <= sizeof f->bytes);
    if(n==2 && data[0]==0x1b && data[1]=='G')
        f->labels_seen_at_end=f->row->label_count;
    memcpy(f->bytes+f->length,data,n);
    f->length+=n;
    ++f->calls;
    return 0;
}
static int32_t encode(void *ctx,const uint8_t *pixels,
                      uint32_t length,uint32_t leading)
{
    io_fixture *f=ctx;
    assert(pixels && length>leading && pixels[leading]!=0);
    ++f->encoded;
    return 0; /* Independent OEM per-line encoder tested elsewhere. */
}
static void bytes(const io_fixture *f,const uint8_t *expected,size_t len)
{
    assert(f->length==len && !memcmp(f->bytes,expected,len));
}
int main(void)
{
    leitz_win2018_label_result result={0};
    leitz_win2018_row_control row={0};
    io_fixture io={0};
    const uint8_t nonwhite[]={0x80};
    const uint8_t white[]={0};

    /* CUSTOMSIZE must set +0x18 so the actual previously
     * reconstructed per-row logic sends ESC w on row 1.
     * At the raster-height boundary it sends ESC f / ESC G. */
    leitz_win2018_label_input custom={
        "CUSTOMSIZE",5,2,1,0,99
    };
    assert(leitz_win2018_initialize_label(&custom,&result,&row)==0);
    assert(result.is_custom_size && row.send_width_flag==1 &&
           row.label_height==2);
    io.row=&row;
    row.row_index=1;
    assert(leitz_win2018_process_counted_row(
        &row,nonwhite,1,capture,encode,&io)==0);
    row.row_index=2;
    assert(leitz_win2018_process_counted_row(
        &row,white,1,capture,encode,&io)==0);
    const uint8_t custom_bytes[]={
        0x1b,'w',0,5, 0x1b,'f',0,1, 0x1b,'G'
    };
    bytes(&io,custom_bytes,sizeof custom_bytes);
    assert(io.calls==3 && io.encoded==1);
    assert(io.labels_seen_at_end==1 && row.label_count==2 &&
           row.row_index==0 && row.previous_trimmed_width==UINT16_MAX);

    /* Continuous + !ModeAuto sends no width prefix and uses
     * iLabelLength instead of the raster page height. */
    memset(&io,0,sizeof io);
    leitz_win2018_label_input manual={
        "Continuous70030001",5,2,2,0,4
    };
    assert(leitz_win2018_initialize_label(&manual,&result,&row)==0);
    assert(!result.is_die_cut && !result.is_custom_size &&
           row.label_height==4 && !row.send_width_flag);
    io.row=&row;
    for(uint16_t i=1;i<=4;++i){
        row.row_index=i;
        assert(leitz_win2018_process_counted_row(
            &row,white,1,capture,encode,&io)==0);
        if(i!=4)assert(io.length==0);
    }
    const uint8_t roll_manual[]={0x1b,'f',0,4, 0x1b,'G'};
    bytes(&io,roll_manual,sizeof roll_manual);
    assert(io.calls==2 && io.encoded==0 &&
           row.label_count==2 && row.row_index==0);

    /* Automatic continuous length follows raster height, not
     * the (deliberately different) caller's iLabelLength. */
    memset(&io,0,sizeof io);
    manual.mode_auto=1;
    assert(leitz_win2018_initialize_label(&manual,&result,&row)==0);
    assert(row.label_height==2);
    io.row=&row;
    for(uint16_t i=1;i<=2;++i){
        row.row_index=i;
        assert(leitz_win2018_process_counted_row(
            &row,white,1,capture,encode,&io)==0);
    }
    const uint8_t roll_auto[]={0x1b,'f',0,2, 0x1b,'G'};
    bytes(&io,roll_auto,sizeof roll_auto);
    assert(row.label_count==2);

    puts("Windows LeitzRp 2018 media init -> ESC w/f/G integration: PASS");
    return 0;
}
