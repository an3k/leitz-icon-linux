#ifndef LEITZ_WIN2018_OEM_CONTINUOUS_H
#define LEITZ_WIN2018_OEM_CONTINUOUS_H

#include "row_control.h"
#include <stdint.h>

/* Original LeitzRp.dll x86/x64 continuous-media path RVA 0x2380.
 * Called AFTER the separate OEM slot-15 dispatcher increments row_index.
 * Portable state adapter, not the original IPrintOemUni2 COM signature.
 * The OEM code also logs debug text and uses an original COM WriteData
 * entry; callbacks here expose exactly the observed command boundaries.
 */

typedef struct {
    leitz_win2018_row_control *row;
    uint16_t paper_height;       /* native row state +0x0c on x64, +0x08 on x86 */
    uint16_t label_limit;        /* native medium +0x0c */
    uint16_t mode_auto;          /* native medium +0x12 */
    uint32_t is_custom_size;    /* native row state +0x18 x64 / +0x14 x86 */
} leitz_win2018_continuous_state;

/* send is the existing row-write callback; encoder is the existing
 * filter's pixel compressor/prefix callback. The OEM checks only an
 * EXACT E_FAIL on flushing queued blank rows, ignores errors for ESC E,
 * ESC G and terminal blank-row ESC f, but forwards the encoder result.
 * NULL/zero divisor inputs return E_FAIL rather than the OEM's unsafe
 * dereference/division. The first 14 lines in each label are suppressed
 * only if is_custom_size is zero, per the original signed remainder.
 * No fixed physical-length limit is added. */
int32_t leitz_win2018_process_continuous_row(
    leitz_win2018_continuous_state *state, const uint8_t *raster,
    uint32_t length, leitz_win2018_row_write send,
    leitz_win2018_row_encode encode_nonblank, void *context);
#endif
