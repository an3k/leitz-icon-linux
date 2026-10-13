#include "classic_process_page_modes_23.h"
#include <string.h>

/* Reconstructed from the previously audited original Mach-O _ProcessPage:
 * 0x1000078eb..0x100007a74 -- blank-line detection / delayed ESC f
 * 0x100007a74..0x100007e69 -- mode 3 offset/width/raw handling
 * 0x100007f32..0x100008882 -- mode 2 offset/width/run handling
 * 0x1000085ba..0x10000887d -- compressed iff shorter than raw row
 * 0x10000815f..0x10000824d -- previous-width and reprint exception.
 *
 * Original data values, branch conditions and emitted bytes are preserved
 * as observed. The C structures expose only recovered state; they do not
 * purport to reproduce the exact manufacturer ABI.
 */
static size_t append_byte(uint8_t *out, size_t offset, uint8_t byte) {
    out[offset] = byte;
    return offset + 1;
}

/* The original _is_bit_set works most-significant-bit first.
 * A run of 1..128 bits emits (length - 1) | (color ? 0x80 : 0).
 * This pass also supports count-only operation; it does not insert a
 * new compressor or alter the original's < raw-size selection.
 */
static size_t run_bits(const uint8_t *row, unsigned count_bytes,
                       uint8_t *out) {
    size_t at = 0;
    unsigned run_length = 0;
    unsigned previous_bit = 0;
    unsigned x;
    for (x=0; x<count_bytes; ++x) {
        int bit;
        for (bit=7; bit>=0; --bit) {
            const unsigned current_bit = (row[x] >> bit) & 1u;
            if (run_length && (previous_bit != current_bit ||
                               run_length == 128u)) {
                if (out) out[at] = (uint8_t)(
                    (previous_bit ? 0x80u : 0u) | (run_length - 1u));
                ++at;
                run_length=0;
            }
            previous_bit = current_bit;
            ++run_length;
        }
    }
    if (run_length) {
        if (out) out[at] = (uint8_t)(
            (previous_bit ? 0x80u : 0u) | (run_length - 1u));
        ++at;
    }
    return at;
}

void classic_mode23_begin(classic_mode23_page *page,
                          classic_mode23_job_cache *job,
                          unsigned mode, unsigned packed_stride,
                          unsigned reprint) {
    page->job=job;
    page->mode=mode;
    page->packed_stride=packed_stride;
    page->previous_left_offset=0;
    page->pending_empty_rows=0;
    page->first_nonblank=1;
    page->reprint=reprint;
}

size_t classic_mode23_row(classic_mode23_page *page,
                          const uint8_t *row, uint8_t *out) {
    const unsigned stride=page->packed_stride;
    unsigned left=0;
    unsigned right=stride;
    unsigned width;
    unsigned *cached_width;
    unsigned emit_width;
    size_t at=0;

    while (left<stride && row[left]==0) ++left;
    if (left==stride) {
        /* The source uses a 16-bit blank-line accumulator. A complete
         * wraparound/overflow behavioral proof remains outstanding. */
        ++page->pending_empty_rows;
        return 0;
    }

    /* 0x100007ab4: mode 3 trims the right edge only when left != 0.
     * Mode 2 trims the right edge on every nonempty row. */
    if (page->mode==2 || left!=0) {
        while (right>left && row[right-1]==0) --right;
    }
    width=right-left;

    if (page->pending_empty_rows) {
        at=append_byte(out,at,0x1b);
        at=append_byte(out,at,'f');
        at=append_byte(out,at,
                       (uint8_t)(page->pending_empty_rows >> 8));
        at=append_byte(out,at,(uint8_t)page->pending_empty_rows);
    }

    /* Mode 2 always emits the left offset. Mode 3 emits it only when
     * the offset changes, with zero as its per-page initial value. */
    if (page->mode==2 || left!=page->previous_left_offset) {
        at=append_byte(out,at,0x1b);
        at=append_byte(out,at,'B');
        at=append_byte(out,at,(uint8_t)left);
    }

    cached_width = page->mode==2 ?
        &page->job->mode2_previous_width :
        &page->job->mode3_previous_width;

    emit_width=(width!=*cached_width) ||
        (page->mode==2 && page->first_nonblank && page->reprint);
    if (emit_width) {
        at=append_byte(out,at,0x1b);
        at=append_byte(out,at,'D');
        at=append_byte(out,at,(uint8_t)width);
    }

    if (page->mode==2 && run_bits(row+left,width,NULL)<width) {
        at=append_byte(out,at,0x17);
        at+=run_bits(row+left,width,out+at);
    } else {
        at=append_byte(out,at,0x16);
        memcpy(out+at,row+left,width);
        at+=width;
    }

    *cached_width=width;
    page->previous_left_offset=left;
    page->pending_empty_rows=0;
    /* The original mode-2 reprint flag is cleared after its width
     * command; normal page calls always pass reprint=0. */
    if (page->mode!=2 || emit_width) page->first_nonblank=0;
    return at;
}
