#include "oem_factory.h"
#include <stddef.h>
#include <string.h>

static const leitz_rp_guid original[] = {
    /* Class at x86 RVA 0x4174, x64 RVA 0x4280:
       6d6abf26-9f38-11d1-882a-00c04fb961ec. */
    {{0x26,0xbf,0x6a,0x6d,0x38,0x9f,0xd1,0x11,
      0x88,0x2a,0x00,0xc0,0x4f,0xb9,0x61,0xec}},
    /* IID_IUnknown */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}},
    /* IID_IClassFactory */
    {{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}},
    /* OEM raster interface at x86 RVA 0x5084, x64 RVA 0x5278. */
    {{0xf0,0xbb,0x7e,0xd6,0xbf,0x78,0xd1,0x11,
      0x94,0x80,0x00,0xa0,0xc9,0x06,0x40,0xb8}}
};

const leitz_rp_guid *leitz_rp_known_guid(leitz_rp_guid_kind kind)
{
    if (kind < LEITZ_RP_GUID_CLSID || kind > LEITZ_RP_GUID_IPRINTOEMUNI)
        return NULL;
    return &original[kind];
}

static int equals(const leitz_rp_guid *guid, leitz_rp_guid_kind kind)
{
    return guid && memcmp(guid, &original[kind], sizeof *guid) == 0;
}

uint32_t leitz_rp_query_interface(leitz_rp_interface current,
    const leitz_rp_guid *iid, leitz_rp_interface *selected)
{
    if (!selected) return LEITZ_RP_FACTORY_E_POINTER;
    *selected = LEITZ_RP_INTERFACE_NONE;
    if (!iid) return LEITZ_RP_FACTORY_E_POINTER;
    if (current == LEITZ_RP_INTERFACE_FACTORY &&
        (equals(iid,LEITZ_RP_GUID_IUNKNOWN) ||
         equals(iid,LEITZ_RP_GUID_ICLASSFACTORY))) {
        *selected = current;
        return LEITZ_RP_FACTORY_S_OK;
    }
    if (current == LEITZ_RP_INTERFACE_RASTER &&
        (equals(iid,LEITZ_RP_GUID_IUNKNOWN) ||
         equals(iid,LEITZ_RP_GUID_IPRINTOEMUNI))) {
        *selected = current;
        return LEITZ_RP_FACTORY_S_OK;
    }
    return LEITZ_RP_FACTORY_E_NOINTERFACE;
}

uint32_t leitz_rp_get_class_object(const leitz_rp_guid *clsid,
    const leitz_rp_guid *iid, int allocated, leitz_rp_interface *selected)
{
    if (!selected) return LEITZ_RP_FACTORY_E_POINTER;
    *selected = LEITZ_RP_INTERFACE_NONE;
    if (!clsid || !iid) return LEITZ_RP_FACTORY_E_POINTER;
    if (!equals(clsid, LEITZ_RP_GUID_CLSID))
        return LEITZ_RP_FACTORY_E_CLASSNOTAVAILABLE;
    if (!allocated) return LEITZ_RP_FACTORY_E_OUTOFMEMORY;
    return leitz_rp_query_interface(LEITZ_RP_INTERFACE_FACTORY,iid,selected);
}

uint32_t leitz_rp_create_instance(int outer_unknown_nonnull,
    const leitz_rp_guid *iid, int allocated, leitz_rp_interface *selected)
{
    if (!selected) return LEITZ_RP_FACTORY_E_POINTER;
    *selected = LEITZ_RP_INTERFACE_NONE;
    if (outer_unknown_nonnull) return LEITZ_RP_FACTORY_E_NOAGGREGATION;
    if (!iid) return LEITZ_RP_FACTORY_E_POINTER;
    if (!allocated) return LEITZ_RP_FACTORY_E_OUTOFMEMORY;
    return leitz_rp_query_interface(LEITZ_RP_INTERFACE_RASTER,iid,selected);
}

uint32_t leitz_rp_lock_server(atomic_int *counter, int lock)
{
    if (!counter) return LEITZ_RP_FACTORY_E_POINTER;
    if (lock) (void)atomic_fetch_add(counter,1);
    else (void)atomic_fetch_sub(counter,1);
    return LEITZ_RP_FACTORY_S_OK;
}

uint32_t leitz_rp_can_unload_now(const atomic_int *flag_a,
    const atomic_int *flag_b)
{
    if (!flag_a || !flag_b) return LEITZ_RP_FACTORY_S_FALSE;
    return atomic_load(flag_a) == 0 && atomic_load(flag_b) == 0 ?
           LEITZ_RP_FACTORY_S_OK : LEITZ_RP_FACTORY_S_FALSE;
}
