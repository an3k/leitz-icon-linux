#ifndef LEITZ_WINDOWS_2018_ROW_CONTROL_H
#define LEITZ_WINDOWS_2018_ROW_CONTROL_H
#include <stddef.h>
#include <stdint.h>

/* Portable C reconstruction of x64 LeitzRp.dll 2.17.4.112 function
 * RVA 0x2060..0x237a, NOT the original COM entry point or object layout.
 * Offsets cited below refer to the OEM's separately owned row-state
 * pointer ([rsi+8] -> rbx), not to this portable C structure.
 */
#define LEITZ_WIN2018_ROW_E_FAIL ((int32_t)0x80004005u)
typedef struct {
    uint16_t previous_trimmed_width; /* OEM state +0x08 */
    uint16_t label_count;            /* OEM state +0x0a */
    uint16_t row_index;              /* OEM state +0x0e, updated by caller */
    uint16_t queued_blank_rows;      /* OEM state +0x10 */
    uint16_t label_height;           /* OEM state +0x12 */
    uint32_t send_width_flag;        /* OEM +0x18 IsCustomSize; ESC w gate */
    uint16_t physical_width;         /* OEM state +0x1c */
    uint16_t medium_switch_flag;    /* OEM medium +0x10 */
} leitz_win2018_row_control;

typedef int32_t (*leitz_win2018_row_write)(void *, const uint8_t *, size_t);
typedef int32_t (*leitz_win2018_row_encode)(void *, const uint8_t *,
                                            uint32_t, uint32_t);

/* One already-counted raster row. Caller owns row_index, mirroring the
 * OEM's separate caller. A nonwhite row uses encode_nonblank; this
 * routine is NOT the unrecovered Windows compression codec.
 * Defined source bounds are checked rather than reproducing unsafe reads.
 */
int32_t leitz_win2018_process_counted_row(
    leitz_win2018_row_control *state, const uint8_t *raster,
    uint32_t length, leitz_win2018_row_write send,
    leitz_win2018_row_encode encode_nonblank, void *context);
#endif
