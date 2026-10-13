#ifndef CLASSIC_RASTER_DITHER_RECOVERED_H
#define CLASSIC_RASTER_DITHER_RECOVERED_H
#include <stdint.h>
/* _DitherPage inner RGB24 conversion recovered from the classic x86-64
 * arithmetic and matrix-selection observations (0x100006dd0..0x100007338).
 * Arguments are an analysis-level interface, NOT the vendor's exact ABI.
 * For OEM RGB24 input only: quality selectors 0..6 and 8..14.
 * Selector 7 is the PPD Draft 1-bit path, not an RGB24 conversion input.
 * Caller supplies exactly as much input/output storage as in the original
 * CUPS page. This reconstruction does not add validation or correction.
 */
void classic_raster_dither_rgb24_recovered(unsigned quality,
              const uint8_t *rgb, unsigned height,
              unsigned bytes_per_line, uint8_t *monochrome);
#endif
