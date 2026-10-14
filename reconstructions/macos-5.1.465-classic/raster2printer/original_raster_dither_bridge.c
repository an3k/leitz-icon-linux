/* Connect the previously reconstructed original _DitherPage to the
 * recovered _raster2printer active page callback interface. */
#include "original_raster_dither_bridge.h"

void classic_original_raster_dither_bridge(
    void *context, classic_oem_read_descriptor *descriptor,
    const uint8_t *rgb, uint8_t *monochrome, unsigned quality) {
    (void)context;
    classic_original_dither_page(descriptor,rgb,monochrome,quality);
}
