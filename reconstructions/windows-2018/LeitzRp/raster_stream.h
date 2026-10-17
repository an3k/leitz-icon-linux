#ifndef LEITZ_WIN2018_RASTER_STREAM_H
#define LEITZ_WIN2018_RASTER_STREAM_H

#include "row_control.h"
#include <stddef.h>
#include <stdint.h>

/* Connects separately reconstructed LeitzRp functions; portable C test
 * adapter, not an OEM COM/Unidrv entry point or a production CUPS backend.
 * Scratch buffers are owned by the caller, distinct from the input row.
 * Both must hold at least the incoming row length; no length is fixed.
 */
typedef struct {
    leitz_win2018_row_control *row;
    leitz_win2018_row_write write;
    void *write_context;
    uint8_t *clipped_row;
    uint8_t *encoded_row;
    size_t scratch_capacity;
    /* Native row controller ignores the FilterGraphics result. Expose it
     * separately for diagnostics without changing the OEM row return. */
    int32_t last_filter_result;
} leitz_win2018_raster_stream;

/* The caller increments row->row_index before processing, exactly as for
 * leitz_win2018_process_counted_row(). A successful label end resets it.
 * Returns the row controller result, not the ignored FilterGraphics result.
 */
int32_t leitz_win2018_stream_counted_row(
    leitz_win2018_raster_stream *stream, const uint8_t *pixels,
    uint32_t row_length);

/* Optional native CONTINUOUS branch adapter (raster slot 15,
 * RVA 0x2380) to the existing static stream_encode(), rather than
 * duplicating the manufacturer's prefix/RLE/physical mask algorithm.
 * Only call for nonwhite rows and with 0 <= leading < length.
 * All source/encoder buffers must be caller-owned and bounded. */
int32_t leitz_win2018_stream_encode_nonblank(
    void *context, const uint8_t *pixels, uint32_t length, uint32_t leading);

#endif
