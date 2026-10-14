#ifndef ORIGINAL_RASTER_DITHER_BRIDGE_H
#define ORIGINAL_RASTER_DITHER_BRIDGE_H
#include "original_dither_page.h"

/* Adapter for classic_oem_page_output_io.dither_page.
 * Reuses the original-derived _DitherPage function; no alternate
 * matrix, raster packing or quality selection is introduced. */
void classic_original_raster_dither_bridge(
    void *context, classic_oem_read_descriptor *descriptor,
    const uint8_t *rgb, uint8_t *monochrome, unsigned quality);
#endif
