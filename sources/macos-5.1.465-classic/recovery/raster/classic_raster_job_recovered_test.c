#include "classic_raster_job_recovered.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned hdr,reads,flushed,progress_calls,terminated;
    unsigned last_requested;
    uint8_t output[8192];
    size_t length;
} scenario;
static unsigned get_header(void *v,classic_page_header *h) {
    scenario *s=v;
    if(++s->hdr>2) return 0;
    h->cups_width=24;h->cups_height=3;
    h->cups_bits_per_pixel=1;
    h->cups_color_space=0;
    h->cups_bytes_per_line=3;
    return 1;
}
static unsigned get_pixels(void *v,void *p,unsigned requested) {
    scenario *s=v;
    const uint8_t sample[9]={0,0,0, 0,0xff,0xff, 0,0,0};
    assert(requested==sizeof sample);
    s->last_requested=requested;
    ++s->reads;
    memcpy(p,sample,sizeof sample);
    return requested;
}
static int write_data(void *v,const uint8_t *p,size_t n) {
    scenario *s=v;
    assert(s->length+n<=sizeof s->output);
    memcpy(s->output+s->length,p,n);
    s->length+=n;
    return 0;
}
static void flush_data(void *v) {++((scenario*)v)->flushed;}
static void report(void *v,unsigned page,unsigned row,unsigned percent) {
    scenario *s=v;
    assert(row==0 && percent==0 && page==s->progress_calls+1);
    ++s->progress_calls;
}
static int cancel(void *v) {return ((scenario*)v)->terminated!=0;}
static unsigned count_bytes(const uint8_t *s,size_t n,
                            const uint8_t *pattern,size_t count) {
    unsigned found=0;
    for(size_t i=0;i+count<=n;++i)
        if(!memcmp(s+i,pattern,count)) ++found;
    return found;
}
int main(void) {
    for(unsigned mode=0;mode<4;++mode) {
        scenario s={0};
        classic_raster_job_io io={{&s,get_header,get_pixels},&s,
                                   write_data,flush_data,report,cancel};
        classic_raster_job_options cfg={0,0,mode,1,0};
        unsigned pages=0;
        assert(classic_raster_job_run(&io,&cfg,&pages)==0);
        assert(pages==2 && s.hdr==3 && s.reads==2);
        assert(s.flushed==2 && s.progress_calls==2);
        assert(s.last_requested==9);
        const uint8_t endpage[4]={0x1b,'G',0x1b,'E'};
        const uint8_t enddocument[4]={0x1b,'@',0x1b,'Z'};
        assert(count_bytes(s.output,s.length,endpage,4)==2);
        assert(!memcmp(s.output+s.length-4,enddocument,4));
        assert(s.output[0]==0x1b && s.output[121]=='@');
        const uint8_t optimized_width[]={0x1b,'D',2};
        if(mode>=2)
            assert(count_bytes(s.output,s.length,optimized_width,3)==1);
    }
    puts("original-derived four-mode job integration checks passed");
    return 0;
}
