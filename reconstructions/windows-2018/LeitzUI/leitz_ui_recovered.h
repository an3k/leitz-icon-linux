#ifndef LEITZ_WIN2018_UI_RECOVERED_H
#define LEITZ_WIN2018_UI_RECOVERED_H
#include <stddef.h>
#include <stdint.h>

/* Offline observable logic recovered independently from LeitzUI.dll x86
 * RVAs 0x1140, 0x1770, 0x17a0 and x64 RVAs 0x1150, 0x17d0,
 * 0x1820. This is NOT the native Windows COM/Unidrv ABI.
 */
#define LEITZ_UI_E_FAIL UINT32_C(0x80004005)
#define LEITZ_UI_SIGNATURE UINT32_C(0x4c504944)
#define LEITZ_UI_CUT_CHANGED_FLAG UINT32_C(0x00000008)

typedef void (*leitz_win2018_ui_last_error_cb)(void *context, uint32_t code);

/* Native GetInfo() conditions, passed as scalar arguments rather than
 * Windows' OEM UI interface parameters. Return 0 = S_OK, E_FAIL otherwise.
 * mode 1: original DWORD signature; mode 3: DWORD 1; mode 4: ERROR_NOT_SUPPORTED;
 * other modes: ERROR_INVALID_PARAMETER. Modes 1/3/4 first report 4-byte size.
 */
uint32_t leitz_win2018_ui_get_info(uint32_t mode, void *output,
    size_t capacity, uint32_t *required_size,
    leitz_win2018_ui_last_error_cb set_last_error, void *context);

/* Selected checkbox/option value and OEM private CutAfterEachLabel DWORD.
 * ui_selected is a 16-bit original OptItem->Sel; native OEM private
 * field resides at x86 +0x10 and x64 +0x18 (pointer-size dependent).
 * The low WORD is what each UI callback reads from OEM private state.
 */
typedef struct {
    uint16_t ui_selected;
    uint32_t oem_cut_after_each_label;
    uint32_t oem_flags;
} leitz_win2018_ui_cut_state;

/* PrinterUI callback: native reason 6 copies OEM Cut setting into Sel.
 * Other reasons do nothing. Always returns 0, as observed.
 */
int leitz_win2018_printer_ui_cut_callback(uint16_t reason,
    leitz_win2018_ui_cut_state *state);

/* DocumentUI callback: reason 6 = OEM to UI (0); reason 10 = APPLY_NOW:
 * UI to OEM, OR private bit 8 and return 1 iff changed; reason 11 =
 * KILLACTIVE: restore UI from OEM, return 0. Other reasons return 0.
 * No UI display, Windows hooks, unsafe native pointers, or transport.
 */
int leitz_win2018_document_ui_cut_callback(uint16_t reason,
    leitz_win2018_ui_cut_state *state);
#endif
