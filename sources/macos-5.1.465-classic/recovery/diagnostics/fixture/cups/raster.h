#ifndef CLASSIC_TEST_RASTER_OFFLINE_CUPS_H
#define CLASSIC_TEST_RASTER_OFFLINE_CUPS_H
/* Test-only CUPS declarations; NOT vendor driver source or production ABI. */
#include <stdint.h>
typedef struct cups_raster_s cups_raster_t;
typedef enum {CUPS_RASTER_READ=0} cups_mode_t;
typedef struct {
    uint32_t cupsHeight;
    uint32_t cupsBytesPerLine;
    uint32_t NumCopies;
    uint32_t cupsBitsPerColor;
    uint32_t cupsBitsPerPixel;
    uint32_t cupsColorOrder;
    uint32_t cupsWidth;
    float cupsBorderlessScalingFactor;
    uint32_t HWResolution[2];
    uint32_t ImagingBoundingBox[4];
    uint32_t Margins[2];
    uint32_t PageSize[2];
    float cupsImagingBBox[4];
    float cupsPageSize[2];
    char cupsPageSizeName[64];
} cups_page_header2_t;
cups_raster_t *cupsRasterOpen(int fd, cups_mode_t mode);
unsigned cupsRasterReadHeader2(cups_raster_t *, cups_page_header2_t *);
unsigned cupsRasterReadPixels(cups_raster_t *, unsigned char *, unsigned);
void cupsRasterClose(cups_raster_t *);
#endif
