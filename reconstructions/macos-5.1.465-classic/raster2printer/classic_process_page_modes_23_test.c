#include "classic_process_page_modes_23.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void eq(const uint8_t *got,size_t n,const uint8_t *want,size_t expected) {
    assert(n==expected);
    assert(memcmp(got,want,n)==0);
}
int main(void) {
    classic_mode23_job_cache job={0};
    classic_mode23_page page;
    uint8_t output[512];
    const uint8_t blank[4]={0,0,0,0};
    const uint8_t dense[4]={0,0xff,0xff,0xff};
    const uint8_t mixed[4]={0,0x55,0x55,0};
    const uint8_t sparse[4]={0,0,0xff,0};
    const uint8_t left_ink[4]={0xff,0,0,0};

    classic_mode23_begin(&page,&job,2,4,0);
    assert(classic_mode23_row(&page,blank,output)==0);
    assert(classic_mode23_row(&page,blank,output)==0);
    const uint8_t rle[]={0x1b,'f',0,2,0x1b,'B',1,0x1b,'D',3,0x17,0x97};
    eq(output,classic_mode23_row(&page,dense,output),rle,sizeof rle);
    assert(job.mode2_previous_width==3);
    const uint8_t raw[]={0x1b,'B',1,0x1b,'D',2,0x16,0x55,0x55};
    eq(output,classic_mode23_row(&page,mixed,output),raw,sizeof raw);
    assert(job.mode2_previous_width==2);

    /* Cached width persists across _ProcessPage calls for separate pages;
     * mode 2 still emits ESC B on every nonempty row. */
    classic_mode23_begin(&page,&job,2,4,0);
    const uint8_t cached_raw[]={0x1b,'B',1,0x16,0x55,0x55};
    eq(output,classic_mode23_row(&page,mixed,output),
       cached_raw,sizeof cached_raw);
    /* Original fourth-argument reprint branch forces width even when cached. */
    classic_mode23_begin(&page,&job,2,4,1);
    eq(output,classic_mode23_row(&page,mixed,output),raw,sizeof raw);

    classic_mode23_begin(&page,&job,3,4,0);
    const uint8_t trimmed[]={0x1b,'B',2,0x1b,'D',1,0x16,0xff};
    eq(output,classic_mode23_row(&page,sparse,output),trimmed,sizeof trimmed);
    assert(classic_mode23_row(&page,blank,output)==0);
    assert(page.pending_empty_rows==1);
    /* Mode 3 keeps its output width cache between pages but starts each
     * page with the left offset reset to zero. */
    classic_mode23_begin(&page,&job,3,4,0);
    const uint8_t trimmed_cached[]={0x1b,'B',2,0x16,0xff};
    eq(output,classic_mode23_row(&page,sparse,output),
       trimmed_cached,sizeof trimmed_cached);
    classic_mode23_begin(&page,&job,3,4,0);
    const uint8_t asymmetry[]={0x1b,'D',4,0x16,0xff,0,0,0};
    eq(output,classic_mode23_row(&page,left_ink,output),
       asymmetry,sizeof asymmetry);

    /* Repeated offset/width on the same mode-3 page suppresses ESC B/D. */
    const uint8_t same[4]={0xff,0x00,0x00,0x00};
    const uint8_t only_data[]={0x16,0xff,0,0,0};
    eq(output,classic_mode23_row(&page,same,output),
       only_data,sizeof only_data);
    assert(page.pending_empty_rows==0);
    /* No automatic flush of a trailing all-white row. */
    assert(classic_mode23_row(&page,blank,output)==0);
    assert(page.pending_empty_rows==1);
    puts("classic ProcessPage modes 2/3 isolated branch checks passed");
    return 0;
}
