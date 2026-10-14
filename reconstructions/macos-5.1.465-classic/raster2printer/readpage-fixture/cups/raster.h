#ifndef OEM_READ_PAGE_TEST_CUPS_RASTER_H
#define OEM_READ_PAGE_TEST_CUPS_RASTER_H
#include <stdint.h>
typedef struct cups_raster_s cups_raster_t;
typedef struct {
    uint32_t cupsWidth,cupsHeight,cupsMediaType,cupsBytesPerLine,
             cupsBitsPerPixel,cupsColorSpace;
    char cupsPageSizeName[64];
} cups_page_header2_t;
unsigned cupsRasterReadHeader2(cups_raster_t *,cups_page_header2_t *);
unsigned cupsRasterReadPixels(cups_raster_t *,unsigned char *,unsigned);
#endif
