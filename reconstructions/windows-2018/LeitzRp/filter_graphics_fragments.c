#include "filter_graphics_fragments.h"
#include <limits.h>

int32_t leitz_win2018_emit_row_prefix(leitz_win2018_row_state *state,
                                     size_t raster_length,
                                     size_t leading_blank_bytes,
                                     leitz_win2018_write_fn write,
                                     void *context)
{
    if (!state || !write || leading_blank_bytes > raster_length ||
        raster_length > UINT32_MAX || leading_blank_bytes > UINT32_MAX)
        return LEITZ_WIN2018_E_FAIL;

    /* Native x64 LeitzRp.dll RVA 0x26df..0x2720:
     * compare unsigned 32-bit cropped width against zero-extended
     * 16-bit previous width at internal state +0x08.
     * Emit ESC D with the LOW 8 bits of width. Only write the 16-bit
     * cached width after the callback does NOT return E_FAIL.
     */
    uint32_t width = (uint32_t)(raster_length - leading_blank_bytes);
    if (state->previous_trimmed_width != width) {
        uint8_t command[] = {0x1b, 'D', (uint8_t)width};
        int32_t result = write(context, command, sizeof command);
        if (result == LEITZ_WIN2018_E_FAIL)
            return result;
        state->previous_trimmed_width = (uint16_t)width;
    }

    /* Native RVA 0x2725..0x274b: unconditional ESC B with LOW
     * 8 bits of the leading blank-byte count. E_FAIL is special.
     */
    uint8_t margin[] = {0x1b, 'B', (uint8_t)leading_blank_bytes};
    int32_t result = write(context, margin, sizeof margin);
    return result == LEITZ_WIN2018_E_FAIL ? result : 0;
}

int32_t leitz_win2018_emit_row_payload(const uint8_t *raw, size_t raw_length,
                                      const uint8_t *compressed,
                                      size_t compressed_length,
                                      leitz_win2018_write_fn write,
                                      void *context)
{
    if (!write || (!raw && raw_length) ||
        (!compressed && compressed_length))
        return LEITZ_WIN2018_E_FAIL;

    /* Native RVA 0x2907..0x29a7: 0x17 only when the encoded data
     * length is STRICTLY less than raw length; otherwise 0x16.
     * Selector and row bytes are separate OEM WriteData calls.
     */
    int choose_compressed = compressed_length < raw_length;
    uint8_t selector = choose_compressed ? 0x17 : 0x16;
    int32_t result = write(context, &selector, 1);
    if (result == LEITZ_WIN2018_E_FAIL)
        return result;
    return choose_compressed
        ? write(context, compressed, compressed_length)
        : write(context, raw, raw_length);
}
