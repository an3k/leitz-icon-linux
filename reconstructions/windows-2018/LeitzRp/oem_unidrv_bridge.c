#include "oem_unidrv_bridge.h"

int32_t leitz_rp_oem_unidrv_slot4_devmode(
    const leitz_rp_oem_unidrv_devmode *args)
{
    if (!args)
        return LEITZ_WIN2018_OEMDEVMODE_E_FAIL;
    /* Do not duplicate or reinterpret any original DEVMODE operation. */
    return leitz_win2018_oemdevmode(
        args->operation, args->source, args->source_capacity,
        args->destination, args->destination_capacity,
        args->published_size, args->callbacks);
}

uint32_t leitz_rp_oem_unidrv_slot6_reset(
    leitz_rp_oem_binding *binding, leitz_rp_release_interface release,
    void *context)
{
    /* Slot 6 and the previously reconstructed destructor cleanup both
     * release this very same cached host interface; reset is not a
     * destructor and does not alter native object ownership. */
    return leitz_rp_oem_release_driver(binding, release, context);
}
