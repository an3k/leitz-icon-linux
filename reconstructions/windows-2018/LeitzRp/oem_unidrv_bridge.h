#ifndef LEITZ_WIN2018_RP_OEM_UNIDRV_BRIDGE_H
#define LEITZ_WIN2018_RP_OEM_UNIDRV_BRIDGE_H

#include "oem_host.h"
#include "../LeitzDevMode/oem_devmode.h"

/* Original LeitzRp.dll raster COM vtable slots 4 and 6, x86/x64.
 * This is a portable bridge to the TWO existing reconstructions,
 * not an implementation of the original COM ABI or a duplicate codec.
 * Slot 4: x86 RVA 0x15d0, x64 RVA 0x15f0 imports
 *          LeitzDevMode.dll!hrOEMDevMode verbatim.
 * Slot 6: x86 RVA 0x1340, x64 RVA 0x1320 releases + clears the
 *          cached host IUnknown at object+8 / object+16.
 */
typedef struct {
    leitz_win2018_oemdm_mode operation;
    const uint8_t *source;
    size_t source_capacity;
    uint8_t *destination;
    size_t destination_capacity;
    uint32_t *published_size;
    const leitz_win2018_oemdm_callbacks *callbacks;
} leitz_rp_oem_unidrv_devmode;

/* Keeps every operation and HRESULT from the existing manufacturer
 * LeitzDevMode reconstruction. The native x86/x64 argument shims
 * are represented by this normalized input, never guessed offsets. */
int32_t leitz_rp_oem_unidrv_slot4_devmode(
    const leitz_rp_oem_unidrv_devmode *args);

/* Reuse the already reconstructed host cached-interface Release;
 * unlike the final object destructor, this does not decrement the
 * raster-object DLL reference counter or free the raster instance. */
uint32_t leitz_rp_oem_unidrv_slot6_reset(
    leitz_rp_oem_binding *binding, leitz_rp_release_interface release,
    void *context);
#endif
