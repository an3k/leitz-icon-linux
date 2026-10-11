/* Test-only ABI-independent shim! NOT bundled into production headers.
   It simulates only the documented CUPS raster API fields used by this filter. */
#ifndef MOCK_CUPS_RASTER_H
#define MOCK_CUPS_RASTER_H
#include <stdint.h>
typedef struct mock_cups_raster_s cups_raster_t;
typedef struct {
    unsigned HWResolution[2];
    unsigned cupsWidth,cupsHeight,cupsBitsPerPixel,cupsBitsPerColor,cupsBytesPerLine;
    unsigned cupsColorOrder,cupsColorSpace;
    float cupsPageSize[2];
} cups_page_header2_t;
#define CUPS_RASTER_READ 0
#define CUPS_ORDER_CHUNKED 0
#define CUPS_CSPACE_W 0
#define CUPS_CSPACE_K 3
cups_raster_t *cupsRasterOpen(int fd,int mode);
unsigned cupsRasterReadHeader2(cups_raster_t *r,cups_page_header2_t *h);
unsigned cupsRasterReadPixels(cups_raster_t *r,unsigned char *p,unsigned len);
void cupsRasterClose(cups_raster_t *r);
#endif
