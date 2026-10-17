#include "ui_driver_interface.h"

static const uint8_t original_driver_iid[16] = {
    0x50, 0x5d, 0xb0, 0x92, 0xbc, 0x78, 0xd1, 0x11,
    0x94, 0x80, 0x00, 0xa0, 0xc9, 0x06, 0x40, 0xb8
};

const uint8_t *leitz_win2018_ui_driver_iid(void)
{
    return original_driver_iid;
}

uint32_t leitz_win2018_ui_publish_driver_interface(
    leitz_win2018_ui_driver_binding *binding,
    void *context, leitz_win2018_ui_query_driver query)
{
    if (!binding) return LEITZ_WIN2018_UI_DRIVER_E_POINTER;
    if (binding->driver_interface) return LEITZ_WIN2018_UI_DRIVER_S_OK;
    if (!query) return LEITZ_WIN2018_UI_DRIVER_E_POINTER;

    void *result = NULL;
    const int32_t hr = query(context, original_driver_iid, &result);
    if (hr < 0) {
        binding->driver_interface = NULL;
        return LEITZ_WIN2018_UI_DRIVER_E_FAIL;
    }
    binding->driver_interface = result;
    return LEITZ_WIN2018_UI_DRIVER_S_OK;
}

uint32_t leitz_win2018_ui_unbind_driver_interface(
    leitz_win2018_ui_driver_binding *binding,
    void *context, leitz_win2018_ui_release_driver release)
{
    if (!binding) return LEITZ_WIN2018_UI_DRIVER_E_POINTER;
    if (!binding->driver_interface) return LEITZ_WIN2018_UI_DRIVER_S_OK;
    if (!release) return LEITZ_WIN2018_UI_DRIVER_E_POINTER;
    void *const stored = binding->driver_interface;
    release(context, stored);
    binding->driver_interface = NULL;
    return LEITZ_WIN2018_UI_DRIVER_S_OK;
}
