#include "oem_row_dispatch.h"
#include "oem_continuous_bridge.h"
#include "label_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {uint8_t bytes[512];size_t size;size_t calls;} sink;
static int32_t collect(void *ctx,const uint8_t *p,size_t n) {
    sink *s=ctx;
    assert(s->size+n<=sizeof s->bytes);
    if(n)memcpy(s->bytes+s->size,p,n);
    s->size+=n;s->calls++;
    return 0;
}
int main(void)
{
    leitz_win2018_row_control row={0};
    leitz_win2018_label_result media={0};
    leitz_win2018_label_input input={"Continuous70030001",16,40,1,0,18};
    sink out={0};uint8_t clipped[8]={0},compressed[8]={0};
    leitz_win2018_raster_stream stream={
        &row,collect,&out,clipped,compressed,sizeof clipped,0
    };
    assert(leitz_win2018_initialize_label(&input,&media,&row)==0);
    assert(media.is_die_cut==0 && row.label_height==18 && row.physical_width==16);
    leitz_win2018_continuous_state continuous={
        .row=&row,.paper_height=media.paper_height,.label_limit=1,
        .mode_auto=input.mode_auto,.is_custom_size=media.is_custom_size
    };
    const uint8_t black[]={0,0xff},white[]={0,0};
    for(unsigned i=1;i<=14;i++){
        assert(leitz_win2018_oem_dispatch_row(
            &stream,black,2,media.is_die_cut,
            leitz_win2018_continuous_alt_row,&continuous)==0);
        assert(row.row_index==i && out.calls==0);
    }
    assert(leitz_win2018_oem_dispatch_row(&stream,white,2,0,
        leitz_win2018_continuous_alt_row,&continuous)==0);
    assert(row.queued_blank_rows==1);
    assert(leitz_win2018_oem_dispatch_row(&stream,black,2,0,
        leitz_win2018_continuous_alt_row,&continuous)==0);
    assert(row.queued_blank_rows==0 && row.row_index==16);
    for(unsigned i=17;i<=18;i++)
        assert(leitz_win2018_oem_dispatch_row(&stream,white,2,0,
            leitz_win2018_continuous_alt_row,&continuous)==0);
    assert(row.queued_blank_rows==0 && row.row_index==18);
    assert(leitz_win2018_oem_dispatch_row(&stream,white,2,0,
        leitz_win2018_continuous_alt_row,&continuous)==0);
    assert(row.label_count==2 && row.row_index==19);
    const uint8_t want[]={
        0x1b,'f',0,1,0x1b,'D',1,0x1b,'B',1,0x16,0xff,
        0x1b,'f',0,2,0x1b,'G'
    };
    if(out.size!=sizeof want || memcmp(out.bytes,want,sizeof want)) {
        fprintf(stderr,"continuous actual %zu expected %zu: ",out.size,sizeof want);
        for(size_t i=0;i<out.size;i++)fprintf(stderr,"%02x ",out.bytes[i]);
        fputc('\n',stderr);
        assert(0);
    }
    assert(leitz_win2018_oem_dispatch_row(&stream,white,2,0,
        leitz_win2018_continuous_alt_row,&continuous)==0);
    assert(out.size==sizeof want); /* no second label beyond limit */
    assert(leitz_win2018_continuous_alt_row(NULL,&stream,black,2)==
           LEITZ_WIN2018_ROW_E_FAIL);
    leitz_win2018_continuous_state bad=continuous;
    leitz_win2018_row_control other={0};bad.row=&other;
    assert(leitz_win2018_continuous_alt_row(&bad,&stream,black,2)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(leitz_win2018_continuous_alt_row(&continuous,NULL,black,2)==
           LEITZ_WIN2018_ROW_E_FAIL);
    puts("Windows OEM continuous 19-row original dispatch + existing RLE: PASS");
    return 0;
}
