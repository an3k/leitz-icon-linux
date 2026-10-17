#ifndef LEITZ_WIN2018_OEM_CONTINUOUS_BRIDGE_H
#define LEITZ_WIN2018_OEM_CONTINUOUS_BRIDGE_H
#include "oem_continuous.h"
#include "raster_stream.h"
/* Actual original continuous-media callback compatible with the existing
 * oem_row_dispatch alternate callback API; all encoding uses the EXISTING
 * raster_stream/filter/RLE code, not a second compression implementation.
 * The host supplies label count and ModeAuto from the OEM medium record.
 */
int32_t leitz_win2018_continuous_alt_row(
    void *context, leitz_win2018_raster_stream *stream,
    const uint8_t *pixels, uint32_t length);
#endif
