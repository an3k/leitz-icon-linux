/* Original macOS classic _DitherPage: independently disassembled. */
#ifndef MAC_CLASSIC_DITHER_H
#define MAC_CLASSIC_DITHER_H
#include <stddef.h>
#include <stdint.h>
typedef struct { unsigned matrix_width; double matrix_divisor; } mac_classic_quality;
/* _raster2printer option chain: case-insensitive original PPD choice -> mode.
 * Draft (mode 7) is 1-bit input and should bypass RGB24 dithering. */
int mac_classic_quality_from_ppd(const char *choice);
int mac_classic_quality_info(unsigned mode, mac_classic_quality *info);
int mac_classic_dither_rgb24(unsigned mode, const uint8_t *rgb, size_t rgb_length,
                             unsigned rows, unsigned rgb_bytes_per_row,
                             uint8_t *bitmap, size_t bitmap_length);
#endif
