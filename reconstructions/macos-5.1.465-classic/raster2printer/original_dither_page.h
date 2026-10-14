#ifndef ORIGINAL_CLASSIC_DITHER_PAGE_H
#define ORIGINAL_CLASSIC_DITHER_PAGE_H
#include "original_read_page.h"
#include <stdint.h>
/* macOS classic OEM _DitherPage (0x100006dd0..0x100007335).
 * Defined RGB24 input and quality modes 0..6,8..14. Quality 7 / unexpected
 * geometry is outside the original function's defined pixel path. */
void classic_original_dither_page(classic_oem_read_descriptor *descriptor,
                                  const uint8_t *rgb,
                                  uint8_t *monochrome,
                                  unsigned quality);
#endif
