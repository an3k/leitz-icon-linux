#ifndef LEITZ_WIN2018_OEM_GDI_CAPS_H
#define LEITZ_WIN2018_OEM_GDI_CAPS_H

#include "label_setup.h"
#include <stddef.h>
#include <stdint.h>

/* Original 2018 Windows LeitzRp.dll media setup:
 * x86 GetDeviceCaps call site RVA 0x195d onward; x64 RVA 0x19a2 onward.
 * Original CreateDCW driver is UTF-16 "WINSPOOL", with a NULL output port.
 * Native Windows GDI function pointers and C++/Unidrv object ABI are not
 * reimplemented. Callers provide the real printer name/DEVMODE and API ops.
 */
#define LEITZ_WIN2018_GDI_CAP_COUNT 13u

typedef void *(*leitz_win2018_gdi_create)(void *context,
    const uint16_t *driver, const uint16_t *printer,
    const uint16_t *port, const void *devmode);
typedef int32_t (*leitz_win2018_gdi_get)(void *context, void *dc,
    int32_t cap_index);
typedef int32_t (*leitz_win2018_gdi_delete)(void *context, void *dc);

typedef struct {
    void *context;
    leitz_win2018_gdi_create create;
    leitz_win2018_gdi_get get;
    leitz_win2018_gdi_delete destroy;
} leitz_win2018_gdi_callbacks;

typedef struct {
    int32_t queried[LEITZ_WIN2018_GDI_CAP_COUNT];
    uint32_t calls;
    uint32_t raster_width;   /* second OEM query, GetDeviceCaps(..., 8) */
    uint32_t raster_height;  /* first OEM query, GetDeviceCaps(..., 10) */
} leitz_win2018_gdi_caps_snapshot;

/* OEM GDI selectors in exact original call order, including the repeated
 * VERTRES query for diagnostics; the first two results alone feed the
 * existing leitz_win2018_initialize_label() input geometry.
 */
const int32_t *leitz_win2018_gdi_original_selectors(void);
const uint16_t *leitz_win2018_gdi_original_driver(void);

/* Validates callback pointers before native-style CreateDCW. An absent DC
 * returns -1, does not call GetDeviceCaps/DeleteDC and leaves outputs
 * unchanged. A valid DC is queried 13 times in the original order, then
 * destroyed exactly once. The original ignores the DeleteDC HRESULT, as
 * does this model. GDI result signs/values are NOT normalized or guessed.
 * The previous media initializer is called, not duplicated.
 * Orientation, stock name and ModeAuto remain authoritative caller input.
 */
int leitz_win2018_gdi_initialize_label(
    const leitz_win2018_gdi_callbacks *gdi,
    const uint16_t *printer_name, const void *win_devmode,
    const leitz_win2018_label_input *source,
    leitz_win2018_label_result *result,
    leitz_win2018_row_control *row,
    leitz_win2018_gdi_caps_snapshot *snapshot);

#endif
