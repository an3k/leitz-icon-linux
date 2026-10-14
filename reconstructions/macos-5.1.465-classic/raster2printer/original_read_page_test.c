#define _POSIX_C_SOURCE 200809L
#include "original_read_page.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cups_raster_s {unsigned headers,reads;};
static struct cups_raster_s fake;
static unsigned result_lengths[]={1,4,0};
unsigned cupsRasterReadHeader2(cups_raster_t *p,cups_page_header2_t *header) {
    assert(p==&fake);
    unsigned count=p->headers++;
    if (count>=3) return 0;
    memset(header,0,sizeof(*header));
    header->cupsWidth=(count==0)?300:704;
    header->cupsHeight=2;
    header->cupsMediaType=13;
    header->cupsBytesPerLine=(count==0)?5:8;
    header->cupsBitsPerPixel=(count==0)?1:24;
    header->cupsColorSpace=(count==0)?0:1;
    strcpy(header->cupsPageSizeName,"CUSTOM_MEDIA");
    return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t *p,unsigned char *buf,unsigned requested) {
    assert(p==&fake && buf!=NULL);
    unsigned i=p->reads++;
    assert(i<3);
    assert(requested==(i==0?10u:16u));
    if (result_lengths[i]) memset(buf,0x5a,result_lengths[i]);
    return result_lengths[i];
}
int main(void) {
    memset(&fake,0,sizeof fake);
    classic_oem_read_descriptor desc={0};desc.cups_raster=&fake;
    cups_page_header2_t h={0};unsigned char *pixel=NULL;
    int bytes=100, pages=0;
    FILE *capture=tmpfile();assert(capture);
    int stderr_copy=dup(fileno(stderr));assert(stderr_copy>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);
    assert(classic_original_read_page(&desc,&h,&pixel,&bytes,&pages)==1);
    assert(pages==1 && bytes==10 && desc.cups_width==300);
    assert(desc.cups_height==2 && desc.packed_bytes_per_line==5);
    assert(desc.cups_bits_per_pixel==1 && desc.cups_color_space==0);
    assert(strcmp(desc.cups_page_size_name,"CUSTOM_MEDIA")==0);
    assert(pixel[0]==0x5a);
    /* Second successful header must free the first allocated page. */
    assert(classic_original_read_page(&desc,&h,&pixel,&bytes,&pages)==1);
    assert(pages==2 && bytes==16 && desc.cups_width==704);
    assert(desc.packed_bytes_per_line==1);
    assert(desc.cups_bits_per_pixel==24 && desc.cups_color_space==1);
    assert(pixel[0]==0x5a);
    /* Zero pixels: original frees buffer, zeroes length but leaves pointer
     * dangling. Do not inspect it or free it a second time. */
    assert(classic_original_read_page(&desc,&h,&pixel,&bytes,&pages)==0);
    assert(pages==3 && bytes==0);
    assert(fake.headers==3 && fake.reads==3);
    assert(classic_original_read_page(&desc,&h,&pixel,&bytes,&pages)==0);
    assert(fake.headers==4 && fake.reads==3 && pages==3);
    assert(fflush(stderr)==0);
    assert(dup2(stderr_copy,fileno(stderr))>=0);assert(close(stderr_copy)==0);
    rewind(capture);
    char diag[4096]={0};size_t n=fread(diag,1,sizeof(diag)-1,capture);diag[n]=0;
    assert(fclose(capture)==0);
    assert(strstr(diag,"raster2printer.c:1049:ReadPage(): cups_page_header, media type/name = 13/'CUSTOM_MEDIA';"));
    assert(strstr(diag,"raster2printer.c:1059:ReadPage(): CUPS BYTES per line : 8"));
    assert(strstr(diag,"raster2printer.c:1061:ReadPage(): BYTES per line : 1"));
    assert(strstr(diag,"raster2printer.c:1089:ReadPage(): \n bytesRead = 1, shouldReadBytes = 10"));
    assert(strstr(diag,"raster2printer.c:1089:ReadPage(): \n bytesRead = 4, shouldReadBytes = 16"));
    /* No success report after the zero-pixel third page. */
    unsigned success_reports=0;
    const char *scan=diag;
    while ((scan=strstr(scan,"bytesRead = "))!=NULL) {++success_reports;++scan;}
    assert(success_reports==2);
    puts("OEM ReadPage headers, stride, realloc, zero-read and stderr checks passed");
    return 0;
}
