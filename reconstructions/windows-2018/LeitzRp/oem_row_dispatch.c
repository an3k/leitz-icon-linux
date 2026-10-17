#include "oem_row_dispatch.h"
#include <stdint.h>

int32_t leitz_win2018_oem_dispatch_row(
    leitz_win2018_raster_stream *stream,
    const uint8_t *pixels, uint32_t length,
    uint32_t is_die_cut,
    leitz_win2018_oem_alt_row alternate, void *alternate_context)
{
    if (!stream || !stream->row || (length && !pixels))
        return LEITZ_WIN2018_ROW_E_FAIL;
    if (!is_die_cut && !alternate)
        return LEITZ_WIN2018_OEM_ALT_PATH_MISSING;

    /* Native x86 RVA 0x1FFF, x64 RVA 0x2044: inc WORD. */
    stream->row->row_index = (uint16_t)(stream->row->row_index + 1u);
    if (is_die_cut)
        return leitz_win2018_stream_counted_row(stream, pixels, length);
    return alternate(alternate_context, stream, pixels, length);
}
