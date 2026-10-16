#include "row_control.h"

static int32_t send_u16(leitz_win2018_row_write send, void *ctx,
                        uint8_t command, uint16_t count)
{
    const uint8_t bytes[4] = {0x1b, command,
                              (uint8_t)(count >> 8), (uint8_t)count};
    return send(ctx, bytes, sizeof bytes);
}

static void end_label(leitz_win2018_row_control *s,
                      leitz_win2018_row_write send, void *ctx)
{
    const uint8_t command[2] = {0x1b, 'G'};
    /* The OEM zeroes DWORD at state+0x0e (row_index and pending rows)
     * and writes 0xffff to state+0x08 BEFORE calling WriteData.
     * It increments state+0x0a AFTER WriteData, irrespective of HRESULT. */
    s->row_index = 0;
    s->queued_blank_rows = 0;
    s->previous_trimmed_width = UINT16_MAX;
    (void)send(ctx, command, sizeof command);
    ++s->label_count; /* native unsigned WORD wraps */
}

int32_t leitz_win2018_process_counted_row(
    leitz_win2018_row_control *s, const uint8_t *raster,
    uint32_t length, leitz_win2018_row_write send,
    leitz_win2018_row_encode encode_nonblank, void *context)
{
    if (!s || !send || !encode_nonblank || (length && !raster))
        return LEITZ_WIN2018_ROW_E_FAIL;

    /* OEM 0x20f2..0x212a: optional ESC w when row_index <= 1.
     * The original does not check this call's HRESULT. */
    if (s->row_index <= 1 && s->send_width_flag)
        (void)send_u16(send, context, 'w', s->physical_width);

    /* OEM 0x215a..0x2187: conditional ESC E before inspecting pixels.
     * The original does not check this call's HRESULT. */
    if (s->medium_switch_flag && s->label_count != 1 && s->row_index == 1) {
        const uint8_t command[] = {0x1b, 'E'};
        (void)send(context, command, sizeof command);
    }

    /* OEM 0x218c..0x21e5: count leading zero bytes. The original
     * includes an unsafe lookahead; bounded inputs are handled safely. */
    uint32_t leading_blank = 0;
    while (leading_blank < length && raster[leading_blank] == 0)
        ++leading_blank;

    if (leading_blank < length) {
        if (s->queued_blank_rows) {
            /* OEM 0x21eb..0x2258: clear pending before checking
             * specifically E_FAIL; other HRESULT values are ignored. */
            int32_t r = send_u16(send, context, 'f', s->queued_blank_rows);
            s->queued_blank_rows = 0;
            if (r == LEITZ_WIN2018_ROW_E_FAIL)
                return r;
        }
        /* OEM 0x225d..0x226d: separate FilterGraphics call.
         * Its HRESULT is not checked by this caller. */
        (void)encode_nonblank(context, raster, length, leading_blank);
    } else {
        /* OEM 0x22b3: even a zero-length row adds one blank line.
         * The original 16-bit counter wraps on overflow. */
        ++s->queued_blank_rows;
        if (s->row_index == s->label_height) {
            (void)send_u16(send, context, 'f', s->queued_blank_rows);
            s->queued_blank_rows = 0;
        }
    }

    /* OEM 0x2272..0x22ae / 0x231e..0x2352: equality, not >=. */
    if (s->row_index == s->label_height)
        end_label(s, send, context);
    return 0;
}
