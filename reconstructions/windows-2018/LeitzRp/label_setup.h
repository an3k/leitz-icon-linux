#ifndef LEITZ_WIN2018_LABEL_SETUP_H
#define LEITZ_WIN2018_LABEL_SETUP_H
#include "row_control.h"
#include <stdint.h>

/* Reconstructed x64 OEM LeitzRp.dll RVA 0x16f7..0x1c4e state setup.
 * All inputs are already-resolved data from Windows Unidrv/GDI:
 * - paper_name: original ASCII paper option, case-sensitive substring
 * - raster_width/height: GetDeviceCaps(HORZRES=8, VERTRES=10)
 * - orientation: DEVMODE dmOrientation (2 = landscape)
 * - mode_auto: original OEM medium WORD +0x12
 * - configured_length: original OEM medium WORD +0x0e.
 * This is a new portable C interface, not the original Windows COM ABI.
 */
typedef struct {
    const char *paper_name;
    uint32_t raster_width;
    uint32_t raster_height;
    int32_t orientation;
    uint16_t mode_auto;
    uint16_t configured_length;
} leitz_win2018_label_input;

typedef struct {
    uint16_t paper_width;     /* OEM state WORD +0x1c */
    uint16_t paper_height;    /* OEM state WORD +0x0c */
    uint16_t label_length;    /* OEM state WORD +0x12 */
    uint32_t is_die_cut;      /* OEM state DWORD +0x14 */
    uint32_t is_custom_size;  /* OEM state DWORD +0x18 */
} leitz_win2018_label_result;

/* Initializes only OEM-derived row fields. The medium-dependent
 * flag from the separate OEM medium +0x10 is not reset or guessed.
 * Returns 0 on success, -1 for null parameters.
 * Like the manufacturer it retains the low 16 bits of raster sizes.
 */
int leitz_win2018_initialize_label(
    const leitz_win2018_label_input *input,
    leitz_win2018_label_result *result,
    leitz_win2018_row_control *row);
#endif
