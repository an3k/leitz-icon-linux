#include "raster_stream.h"
#include "filter_graphics_fragments.h"
#include "rle_windows_2018.h"

static int32_t stream_write(void *context, const uint8_t *data, size_t length)
{
    leitz_win2018_raster_stream *s = context;
    return s->write(s->write_context, data, length);
}

static int32_t stream_encode(void *context, const uint8_t *pixels,
                             uint32_t length, uint32_t leading)
{
    leitz_win2018_raster_stream *s = context;
    leitz_win2018_row_state prefix = {s->row->previous_trimmed_width};
    size_t encoded_length = 0;
    const size_t payload_length = (size_t)length - leading;

    /* The OEM conditional final-byte mask occurs in FilterGraphics,
     * after the row controller has already counted leading zeroes.
     * Make a copy instead of mutating the source raster. */
    if (leitz_win2018_copy_clipped_row(
            pixels, length, s->row->physical_width,
            s->clipped_row, s->scratch_capacity) != 0) {
        s->last_filter_result = LEITZ_WIN2018_E_FAIL;
        return s->last_filter_result;
    }
    int32_t result = leitz_win2018_emit_row_prefix(
        &prefix, length, leading, stream_write, s);
    /* Prefix caching is part of the shared native row state even if the
     * ESC B write failed after a successful ESC D width update. */
    s->row->previous_trimmed_width = prefix.previous_trimmed_width;
    if (result == LEITZ_WIN2018_E_FAIL) {
        s->last_filter_result = result;
        return result;
    }
    const uint8_t *payload = s->clipped_row + leading;
    int rc = leitz_win2018_rle_encode(
        payload, payload_length, s->encoded_row, s->scratch_capacity,
        &encoded_length);
    if (rc == LEITZ_WIN2018_RLE_INVALID) {
        s->last_filter_result = LEITZ_WIN2018_E_FAIL;
        return s->last_filter_result;
    }
    const int use_compression = rc == LEITZ_WIN2018_RLE_COMPRESSED;
    result = leitz_win2018_emit_row_payload(
        payload, payload_length,
        use_compression ? s->encoded_row : payload,
        use_compression ? encoded_length : payload_length,
        stream_write, s);
    s->last_filter_result = result;
    return result;
}

int32_t leitz_win2018_stream_counted_row(
    leitz_win2018_raster_stream *s, const uint8_t *pixels,
    uint32_t row_length)
{
    if (!s || !s->row || !s->write || !s->row->physical_width ||
        (row_length && (!pixels || !s->clipped_row || !s->encoded_row ||
                        s->scratch_capacity < row_length ||
                        s->clipped_row == s->encoded_row)))
        return LEITZ_WIN2018_ROW_E_FAIL;
    s->last_filter_result = 0;
    return leitz_win2018_process_counted_row(
        s->row, pixels, row_length, stream_write, stream_encode, s);
}

/* Additive bridge: expose the ALREADY reconstructed FilterGraphics
 * implementation to the native continuous-media controller without
 * copying its RLE/prefix/last-byte-mask code. */
int32_t leitz_win2018_stream_encode_nonblank(
    void *context, const uint8_t *pixels, uint32_t length, uint32_t leading)
{
    leitz_win2018_raster_stream *s=context;
    if (!s || !s->row || !s->row->physical_width || !s->write ||
        !s->clipped_row || !s->encoded_row ||
        (length && !pixels) || leading>=length ||
        s->scratch_capacity<length || s->clipped_row==s->encoded_row)
        return LEITZ_WIN2018_ROW_E_FAIL;
    s->last_filter_result=0;
    return stream_encode(s,pixels,length,leading);
}
