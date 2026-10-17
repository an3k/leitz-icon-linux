#include "leitz_ui_recovered.h"

static uint32_t error(uint32_t code, leitz_win2018_ui_last_error_cb cb,
                      void *context)
{
    if (cb) cb(context, code);
    return LEITZ_UI_E_FAIL;
}

uint32_t leitz_win2018_ui_get_info(uint32_t mode, void *output,
    size_t capacity, uint32_t *required_size,
    leitz_win2018_ui_last_error_cb set_last_error, void *context)
{
    if (!required_size || (mode != 1u && mode != 3u && mode != 4u))
        return error(87u, set_last_error, context);
    *required_size = 4u;
    if (!output || capacity < 4u)
        return error(122u, set_last_error, context);
    if (mode == 4u) {
        *required_size = 0u;
        return error(50u, set_last_error, context);
    }
    const uint32_t value = mode == 1u ? LEITZ_UI_SIGNATURE : 1u;
    unsigned char *dest = (unsigned char *)output;
    for (unsigned i = 0; i < 4; ++i) dest[i] = (unsigned char)(value >> (8u*i));
    return 0;
}

int leitz_win2018_printer_ui_cut_callback(uint16_t reason,
    leitz_win2018_ui_cut_state *state)
{
    if (!state) return 0;
    if (reason == 6u)
        state->ui_selected = (uint16_t)state->oem_cut_after_each_label;
    return 0;
}

int leitz_win2018_document_ui_cut_callback(uint16_t reason,
    leitz_win2018_ui_cut_state *state)
{
    if (!state) return 0;
    if (reason == 6u || reason == 11u) {
        state->ui_selected = (uint16_t)state->oem_cut_after_each_label;
        return 0;
    }
    if (reason == 10u &&
        state->ui_selected != (uint16_t)state->oem_cut_after_each_label) {
        state->oem_cut_after_each_label = (uint32_t)state->ui_selected;
        state->oem_flags |= LEITZ_UI_CUT_CHANGED_FLAG;
        return 1;
    }
    return 0;
}
