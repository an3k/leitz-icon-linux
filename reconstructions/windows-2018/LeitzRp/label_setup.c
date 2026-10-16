#include "label_setup.h"
#include <limits.h>
#include <string.h>

int leitz_win2018_initialize_label(
    const leitz_win2018_label_input *in,
    leitz_win2018_label_result *out,
    leitz_win2018_row_control *row)
{
    if (!in || !in->paper_name || !out || !row)
        return -1;

    /* OEM x64 0x16fa..0x1701 and 0x1793 reset data before reading
     * paper name: previous width 0xffff, initial label count one,
     * current/pending rows zero, IsDieCut one, IsCustomSize zero.
     * Original wcsstr at 0x18c2/0x18dd searches substrings, not
     * whole-paper tokens. All documented GPD option keys are ASCII. */
    const uint32_t is_die_cut =
        strstr(in->paper_name, "Continuous") ? 0u : 1u;
    const uint32_t is_custom_size =
        strstr(in->paper_name, "CUSTOMSIZE") ? 1u : 0u;

    /* OEM 0x1b33..0x1bd7: swapping the two GetDeviceCaps pixel
     * dimensions occurs ONLY for landscape (dmOrientation=2) AND
     * either IsCustomSize or IsDieCut. Non-diecut continuous stock
     * does not take this swap branch even if orientation is 2. */
    const int rotated = in->orientation == 2 &&
                        (is_custom_size || is_die_cut);
    const uint16_t paper_width = (uint16_t)(
        rotated ? in->raster_height : in->raster_width);
    const uint16_t paper_height = (uint16_t)(
        rotated ? in->raster_width : in->raster_height);

    /* OEM 0x1b7a..0x1bab and 0x1c12..0x1c3b:
     * pOEMDEV->ModeAuto at OEM medium +0x12, plus custom/diecut
     * flags, determine whether iLabelLength at OEM medium +0x0e
     * overrides the selected page/raster height. No global
     * fixed maximum print length or arbitrary 170-mm cap. */
    const uint16_t label_length =
        (!in->mode_auto && !is_custom_size && !is_die_cut)
        ? in->configured_length : paper_height;

    leitz_win2018_label_result value = {
        paper_width, paper_height, label_length, is_die_cut, is_custom_size
    };
    *out = value;

    row->previous_trimmed_width = UINT16_MAX;
    row->label_count = 1u;
    row->row_index = 0u;
    row->queued_blank_rows = 0u;
    row->label_height = label_length;
    row->physical_width = paper_width;
    /* Existing row controller checks OEM DWORD +0x18 for ESC w.
     * It is IsCustomSize (not a general width-change signal). */
    row->send_width_flag = is_custom_size;
    /* OEM medium +0x10 is outside this object; leave its separate
     * row controller value untouched rather than inventing data. */
    return 0;
}
