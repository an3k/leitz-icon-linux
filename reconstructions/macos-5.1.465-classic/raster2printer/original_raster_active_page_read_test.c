#define _POSIX_C_SOURCE 200809L
#include "original_raster_active_page_read.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cups_raster_s {unsigned headers,reads;};
static struct cups_raster_s raster;
unsigned cupsRasterReadHeader2(cups_raster_t *r,cups_page_header2_t *hdr) {
    assert(r==&raster);
    ++raster.headers;
    memset(hdr,0,sizeof *hdr);
    if (raster.headers==3) return 0;
    hdr->cupsWidth=raster.headers==1?300u:88u;
    hdr->cupsHeight=raster.headers==1?2u:1u;
    hdr->cupsMediaType=27;
    hdr->cupsBytesPerLine=raster.headers==1?12u:6u;
    hdr->cupsBitsPerPixel=raster.headers==1?24u:1u;
    hdr->cupsColorSpace=raster.headers==1?1u:0u;
    strcpy(hdr->cupsPageSizeName,"CUSTOM_88");
    return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t *r,unsigned char *data,unsigned len) {
    assert(r==&raster);
    ++raster.reads;
    if(raster.reads==1) {
        assert(data!=NULL && len==24u);
        memset(data,0xab,5);
        return 5; /* positive short read is still an OEM page */
    }
    assert(raster.reads==2 && data!=NULL && len==6u);
    return 0; /* no dither allocation; original page buffer remains live */
}
int main(void) {
    /* SHA256-verified OEM _raster2printer disassembly contains
     * unsigned divl at 0x1000057eb and signed idivl at 0x100005800.
     * The exported _ReadPage uses the same pair of operations.
     * These are calculation-only tests; no huge allocation. */
    assert(classic_oem_read_page_packed_stride(0x80000008u,24u)==89478485u);
    assert(classic_oem_read_page_packed_stride(0x80000010u,24u)==89478487u);
    classic_oem_read_descriptor d={0};
    d.cups_raster=&raster;
    cups_page_header2_t h={0};
    classic_oem_active_page_state s={0};
    FILE *capture=tmpfile();assert(capture);
    int fd=dup(fileno(stderr));assert(fd>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);

    assert(classic_original_raster_active_page_read(&d,&h,&s)==1);
    assert(s.page_number==1 && s.requested_bytes==24);
    assert(s.received_bytes==5 && s.page_buffer!=NULL && s.dithered_page!=NULL);
    assert(s.page_buffer[0]==0xab && s.page_buffer[4]==0xab);
    assert(d.cups_height==2 && d.cups_width==300);
    assert(d.cups_bytes_per_line==12 && d.packed_bytes_per_line==1);
    assert(d.cups_color_space==1 && d.cups_bits_per_pixel==24);
    assert(strcmp(d.cups_page_size_name,"CUSTOM_88")==0);
    free(s.dithered_page);
    free(s.page_buffer);
    s.dithered_page=NULL;
    s.page_buffer=NULL;

    assert(classic_original_raster_active_page_read(&d,&h,&s)==0);
    assert(s.page_number==2 && s.requested_bytes==0);
    assert(s.received_bytes==0 && s.page_buffer!=NULL);
    assert(s.dithered_page==NULL && d.packed_bytes_per_line==6);
    free(s.page_buffer);
    s.page_buffer=NULL;

    assert(classic_original_raster_active_page_read(&d,&h,&s)==0);
    assert(s.page_number==2 && s.requested_bytes==0);
    assert(s.page_buffer==NULL);
    assert(raster.headers==3 && raster.reads==2);

    assert(fflush(stderr)==0);
    assert(dup2(fd,fileno(stderr))>=0 && close(fd)==0);
    rewind(capture);
    char output[4096]={0};
    size_t n=fread(output,1,sizeof(output)-1,capture);output[n]=0;
    assert(fclose(capture)==0);
    assert(strstr(output,
        "\nraster2printer.c:300:raster2printer(): cups_page_header, media type/name = 27/'CUSTOM_88';"));
    assert(strstr(output,
        "\nraster2printer.c:310:raster2printer(): CupsPageSizeName : CUSTOM_88"));
    assert(strstr(output,
        "\nraster2printer.c:312:raster2printer(): CUPS BYTES per line : 12\n"));
    assert(strstr(output,
        "\nraster2printer.c:314:raster2printer(): BYTES per line : 1\n"));
    assert(strstr(output,
        "\nraster2printer.c:322:raster2printer(): SUCCESS malloc pageBuffer : 24\n"));
    assert(strstr(output,
        "\nraster2printer.c:340:raster2printer():  bytesRead = 5, shouldReadBytes = 24\n"));
    assert(strstr(output,
        "\nraster2printer.c:356:raster2printer(): SUCCESS malloc ditheredPage : 24\n"));
    assert(strstr(output,
        "\nraster2printer.c:322:raster2printer(): SUCCESS malloc pageBuffer : 6\n"));
    assert(strstr(output,
        "\nraster2printer.c:346:raster2printer(): No more available page : 0\n"));
    /* No second allocation of ditheredPage when the pixel read returned 0. */
    const char *p=output;unsigned dither_reports=0,read_reports=0;
    while ((p=strstr(p,"SUCCESS malloc ditheredPage"))!=NULL) {++dither_reports;++p;}
    p=output;
    while ((p=strstr(p,"bytesRead = "))!=NULL) {++read_reports;++p;}
    assert(dither_reports==1 && read_reports==1);
    puts("OEM inline raster page read/malloc/short-read/EOF branches passed");
    return 0;
}
