#include "oem_continuous.h"
#include <stddef.h>

static int32_t emit_u16(leitz_win2018_row_write send, void *context,
                        uint8_t command, uint16_t value)
{
    const uint8_t buffer[] = {0x1b, command,
                              (uint8_t)(value >> 8), (uint8_t)value};
    return send(context, buffer, sizeof buffer);
}

int32_t leitz_win2018_process_continuous_row(
    leitz_win2018_continuous_state *state, const uint8_t *raster,
    uint32_t length, leitz_win2018_row_write send,
    leitz_win2018_row_encode encode_nonblank, void *context)
{
    if (!state || !state->row || !send || !encode_nonblank ||
        !state->row->label_height || (length && !raster))
        return LEITZ_WIN2018_ROW_E_FAIL;
    leitz_win2018_row_control *row = state->row;

    /* x64 0x2440-0x246b, x86 0x2447-0x2473: not checked. */
    if (row->label_count != 1u && row->row_index == 1u) {
        const uint8_t bytes[] = {0x1b, 'E'};
        (void)send(context, bytes, sizeof bytes);
    }

    /* OEM stops at the original requested label count; the caller
     * already incremented row_index. x64 0x246b, x86 0x2473. */
    if (row->label_count > state->label_limit)
        return 0;

    /* x64 0x2476-0x248c, x86 0x2487-0x249b: zero 32 bits at
     * +0x0e (row_index and queued_blank_rows), cache=0xffff. */
    if (row->row_index == state->paper_height) {
        row->row_index = 0;
        row->queued_blank_rows = 0;
        row->previous_trimmed_width = UINT16_MAX;
    }

    /* A complete length closes on the NEXT counted row. OEM does not
     * use >= and does not cut immediately on the last pixel row.
     * x64 0x2490-0x24f3 / x86 0x249f-0x2508. */
    if (state->row->label_height < row->row_index) {
        row->queued_blank_rows = 0;
        row->previous_trimmed_width = UINT16_MAX;
        if ((uint32_t)state->row->label_height + 1u == row->row_index) {
            const uint8_t bytes[] = {0x1b, 'G'};
            (void)send(context, bytes, sizeof bytes);
            ++row->label_count; /* OEM unsigned 16-bit count wraps. */
        }
        return 0;
    }

    /* Original signed idiv: (row_index-1) % label_height; reject the
     * initial 0..13 remainder when IsCustomSize DWORD == 0.
     * x64 0x24fa-0x250e / x86 0x250d-0x251a. */
    if (!state->is_custom_size &&
        (((int32_t)row->row_index - 1) % (int32_t)row->label_height) < 14)
        return 0;

    uint32_t leading=0;
    while (leading < length && raster[leading] == 0)
        ++leading;

    if (leading < length) {
        if (row->queued_blank_rows) {
            const int32_t result = emit_u16(send, context, 'f',
                                             row->queued_blank_rows);
            row->queued_blank_rows = 0; /* before HRESULT comparison */
            if (result == LEITZ_WIN2018_ROW_E_FAIL)
                return result;
        }
        /* OEM forwards FilterGraphics return in the continuous path;
         * unlike the already recovered die-cut row controller. */
        return encode_nonblank(context, raster, length, leading);
    }

    ++row->queued_blank_rows; /* OEM 16-bit overflow retained. */
    if (row->row_index == row->label_height && !state->mode_auto) {
        (void)emit_u16(send, context, 'f', row->queued_blank_rows);
        row->queued_blank_rows = 0;
    }
    return 0;
}
