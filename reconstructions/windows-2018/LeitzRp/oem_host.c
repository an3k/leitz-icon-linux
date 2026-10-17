#include "oem_host.h"

static const uint8_t original_rp_iid[16] = {
    0xf1,0xbb,0x7e,0xd6,0xbf,0x78,0xd1,0x11,
    0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8
};

static uint32_t fail(leitz_rp_set_last_error cb, void *ctx, uint32_t code)
{
    if (cb) cb(ctx, code);
    return LEITZ_RP_E_FAIL;
}

uint32_t leitz_rp_oem_get_info(uint32_t mode, uint8_t *output,
                              size_t capacity, uint32_t *required,
                              leitz_rp_set_last_error error_cb, void *ctx)
{
    if (!required) return fail(error_cb, ctx, 87u);
    if (mode != 1u && mode != 3u && mode != 4u)
        return fail(error_cb, ctx, 87u);
    if (mode == 4u) {
        *required = 8u;
        return LEITZ_RP_E_FAIL;
    }
    *required = 4u;
    if (!output || capacity < 4u) return fail(error_cb, ctx, 122u);
    const uint32_t value = mode == 1u ? UINT32_C(0x4c504944) : 1u;
    for (unsigned i = 0; i < 4; ++i)
        output[i] = (uint8_t)(value >> (8u * i));
    return LEITZ_RP_S_OK;
}

const uint8_t *leitz_rp_oem_driver_iid(void)
{
    return original_rp_iid;
}

uint32_t leitz_rp_oem_publish_driver(leitz_rp_oem_binding *binding,
                                    leitz_rp_host_query_fn query, void *ctx)
{
    if (!binding) return LEITZ_RP_E_POINTER;
    if (binding->driver_interface) return LEITZ_RP_S_OK;
    if (!query) return LEITZ_RP_E_POINTER;
    void *value = NULL;
    const int32_t hr = query(ctx, original_rp_iid, &value);
    if (hr < 0) {
        binding->driver_interface = NULL;
        return LEITZ_RP_E_FAIL;
    }
    binding->driver_interface = value;
    return LEITZ_RP_S_OK;
}

uint32_t leitz_rp_oem_release_driver(leitz_rp_oem_binding *binding,
                                    leitz_rp_release_interface release, void *ctx)
{
    if (!binding) return LEITZ_RP_E_POINTER;
    if (!binding->driver_interface) return LEITZ_RP_S_OK;
    if (!release) return LEITZ_RP_E_POINTER;
    void *const stored = binding->driver_interface;
    release(ctx, stored);
    binding->driver_interface = NULL;
    return LEITZ_RP_S_OK;
}
