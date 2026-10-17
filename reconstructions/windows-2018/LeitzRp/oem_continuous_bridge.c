#include "oem_continuous_bridge.h"

static int32_t stream_write_callback(void *context,
                                     const uint8_t *bytes, size_t length)
{
    leitz_win2018_raster_stream *stream=context;
    return stream->write(stream->write_context,bytes,length);
}

int32_t leitz_win2018_continuous_alt_row(
    void *context, leitz_win2018_raster_stream *stream,
    const uint8_t *pixels, uint32_t length)
{
    leitz_win2018_continuous_state *state=context;
    if (!stream || !state || !stream->row || stream->row != state->row ||
        !stream->write || (length && (!pixels || !stream->clipped_row ||
        !stream->encoded_row || stream->scratch_capacity < length ||
        stream->clipped_row == stream->encoded_row)))
        return LEITZ_WIN2018_ROW_E_FAIL;
    return leitz_win2018_process_continuous_row(
        state,pixels,length,stream_write_callback,
        leitz_win2018_stream_encode_nonblank,stream);
}
