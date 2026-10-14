#define _POSIX_C_SOURCE 200809L
#include "original_dither_page.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static unsigned seen_quality,seen_height,seen_stride,called;
/* Offline linkage proves diagnostics + argument/geometry delegation. Real
 * matrix-driven RGB24 algorithm is separately compiled and tested by CI. */
void classic_raster_dither_rgb24_recovered(unsigned quality,
          const uint8_t *rgb,unsigned height,unsigned stride,uint8_t *out) {
    assert(rgb[0]==0x33);++called;
    seen_quality=quality;seen_height=height;seen_stride=stride;
    out[0]=0xa5;
}
int main(void) {
    classic_oem_read_descriptor d={0};
    d.cups_height=7;d.cups_bytes_per_line=27;
    d.packed_bytes_per_line=2;
    unsigned char rgb[27]={0x33},bw[8]={0};
    FILE *f=tmpfile();assert(f);
    int saved=dup(fileno(stderr));assert(saved>=0);
    assert(dup2(fileno(f),fileno(stderr))>=0);
    classic_original_dither_page(&d,rgb,bw,11);
    assert(called==1 && seen_quality==11 && seen_height==7 && seen_stride==27);
    assert(bw[0]==0xa5);
    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0);assert(close(saved)==0);
    rewind(f);char msg[256]={0};size_t n=fread(msg,1,255,f);msg[n]=0;
    assert(fclose(f)==0);
    assert(strcmp(msg,
           "\nraster2printer.c:593:DitherPage(): Height = 7"
           "\nraster2printer.c:594:DitherPage(): Bytes per line = 2")==0);
    puts("Classic OEM DitherPage geometry and exact diagnostic checks passed");
    return 0;
}
