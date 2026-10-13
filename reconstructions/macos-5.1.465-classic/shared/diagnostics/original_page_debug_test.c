#define _POSIX_C_SOURCE 200809L
#include <cups/raster.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void PrintPageDebugInfo(cups_page_header2_t *header);

int main(void) {
    cups_page_header2_t h={0};
    h.cupsBytesPerLine=44;h.cupsBitsPerColor=8;h.cupsBitsPerPixel=24;
    h.cupsColorOrder=0;h.cupsWidth=88;h.cupsHeight=40;
    h.cupsBorderlessScalingFactor=1.25f;
    h.HWResolution[0]=300;h.HWResolution[1]=600;
    h.ImagingBoundingBox[0]=1;h.ImagingBoundingBox[1]=2;
    h.ImagingBoundingBox[2]=3;h.ImagingBoundingBox[3]=4;
    h.Margins[0]=5;h.Margins[1]=6;
    h.PageSize[0]=7;h.PageSize[1]=8;
    h.cupsImagingBBox[0]=1.0f;h.cupsImagingBBox[1]=2.0f;
    h.cupsImagingBBox[2]=3.0f;h.cupsImagingBBox[3]=4.0f;
    h.cupsPageSize[0]=5.0f;h.cupsPageSize[1]=6.0f;
    memcpy(h.cupsPageSizeName,"USER",5);
    FILE *capture=tmpfile();assert(capture);
    int fd=dup(fileno(stderr));assert(fd>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);
    PrintPageDebugInfo(&h);
    assert(fflush(stderr)==0);
    assert(dup2(fd,fileno(stderr))>=0);assert(close(fd)==0);
    rewind(capture);
    char out[2048]={0};size_t n=fread(out,1,sizeof(out)-1,capture);
    out[n]=0;fclose(capture);
    const char *expected=
       "\nraster_c_functions.c:385:PrintPageDebugInfo(): cupsBytesPerLine = 44"
       "\nraster_c_functions.c:386:PrintPageDebugInfo(): cupsBitsPerColor = 8"
       "\nraster_c_functions.c:387:PrintPageDebugInfo(): cupsBitsPerPixel = 24"
       "\nraster_c_functions.c:388:PrintPageDebugInfo(): cupsColorOrder = 0"
       "\nraster_c_functions.c:389:PrintPageDebugInfo(): cupsWidth=88; .cupsHeight = 40; .cupsBorderlessScalingFactor=1.25000"
       "\nraster_c_functions.c:391:PrintPageDebugInfo(): HWResolution=[300, 600]; .ImagingBoundingBox=[1, 2, 3, 4]; .Margins=[5, 6]; .PageSize=[7, 8]; .cupsImagingBBox=[1.00000, 2.00000, 3.00000, 4.00000]; cupsPageSize=[5.00000, 6.00000]"
       "\nraster_c_functions.c:392:PrintPageDebugInfo(): cupsPageSizeName='USER';";
    assert(strcmp(out,expected)==0);
    puts("macOS classic original PrintPageDebugInfo format checked");
    return 0;
}
