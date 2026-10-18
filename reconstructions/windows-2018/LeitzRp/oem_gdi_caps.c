#include "oem_gdi_caps.h"

/* Native Windows GDI GetDeviceCaps selectors, directly observed from both
 * original LeitzRp.dll architectures. 10 = VERTRES, 8 = HORZRES;
 * remaining values are the manufacturer's diagnostic probes.
 */
static const int32_t original_selectors[LEITZ_WIN2018_GDI_CAP_COUNT] = {
    10, 8, 10, 4, 6, 88, 90, 40, 42, 110, 111, 112, 113
};
static const uint16_t original_driver[] = {
    'W','I','N','S','P','O','O','L',0
};

const int32_t *leitz_win2018_gdi_original_selectors(void)
{
    return original_selectors;
}
const uint16_t *leitz_win2018_gdi_original_driver(void)
{
    return original_driver;
}

int leitz_win2018_gdi_initialize_label(
    const leitz_win2018_gdi_callbacks *gdi,
    const uint16_t *printer_name, const void *win_devmode,
    const leitz_win2018_label_input *source,
    leitz_win2018_label_result *result,
    leitz_win2018_row_control *row,
    leitz_win2018_gdi_caps_snapshot *snapshot)
{
    if (!gdi || !gdi->create || !gdi->get || !gdi->destroy ||
        !printer_name || !source || !source->paper_name ||
        !result || !row)
        return -1;

    void *dc = gdi->create(gdi->context, original_driver,
                           printer_name, NULL, win_devmode);
    if (!dc)
        return -1;

    leitz_win2018_gdi_caps_snapshot obtained = {0};
    for (size_t i = 0; i < LEITZ_WIN2018_GDI_CAP_COUNT; ++i) {
        obtained.queried[i] = gdi->get(gdi->context, dc,
                                         original_selectors[i]);
        ++obtained.calls;
    }
    /* Original x86/x64 does not gate the geometry on positive return
     * values; low WORD truncation happens in the existing initializer. */
    obtained.raster_height = (uint32_t)obtained.queried[0];
    obtained.raster_width = (uint32_t)obtained.queried[1];
    (void)gdi->destroy(gdi->context, dc);

    leitz_win2018_label_input resolved = *source;
    resolved.raster_height = obtained.raster_height;
    resolved.raster_width = obtained.raster_width;
    const int code = leitz_win2018_initialize_label(&resolved,result,row);
    if (code == 0 && snapshot)
        *snapshot = obtained;
    return code;
}
