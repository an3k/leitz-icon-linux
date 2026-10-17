#ifndef LEITZ_WIN2018_RP_OEM_FACTORY_H
#define LEITZ_WIN2018_RP_OEM_FACTORY_H
#include <stdint.h>
#include <stdatomic.h>

/* Original LeitzRp.dll x86/x64 DllGetClassObject and IClassFactory
 * decisions. This is NOT a native Windows COM object or vtable. */
#define LEITZ_RP_FACTORY_S_OK UINT32_C(0)
#define LEITZ_RP_FACTORY_S_FALSE UINT32_C(1)
#define LEITZ_RP_FACTORY_E_POINTER UINT32_C(0x80004003)
#define LEITZ_RP_FACTORY_E_NOINTERFACE UINT32_C(0x80004002)
#define LEITZ_RP_FACTORY_E_OUTOFMEMORY UINT32_C(0x8007000e)
#define LEITZ_RP_FACTORY_E_NOAGGREGATION UINT32_C(0x80040110)
#define LEITZ_RP_FACTORY_E_CLASSNOTAVAILABLE UINT32_C(0x80040111)

typedef struct { uint8_t bytes[16]; } leitz_rp_guid;
typedef enum {
    LEITZ_RP_GUID_CLSID = 0,
    LEITZ_RP_GUID_IUNKNOWN = 1,
    LEITZ_RP_GUID_ICLASSFACTORY = 2,
    LEITZ_RP_GUID_IPRINTOEMUNI = 3
} leitz_rp_guid_kind;
typedef enum {
    LEITZ_RP_INTERFACE_NONE = 0,
    LEITZ_RP_INTERFACE_FACTORY = 1,
    LEITZ_RP_INTERFACE_RASTER = 2
} leitz_rp_interface;

/* Exact original Windows in-memory GUID bytes (not ASCII GUID strings).
 * OEM raster IID is d67ebbf0-78bf-11d1-9480-00a0c90640b8.
 * IMPORTANT: This is distinct from d67ebbf1, the IID requested by
 * the separate PublishDriverInterface handler in oem_host.c. */
const leitz_rp_guid *leitz_rp_known_guid(leitz_rp_guid_kind kind);

uint32_t leitz_rp_query_interface(leitz_rp_interface current,
    const leitz_rp_guid *iid, leitz_rp_interface *selected);
/* Original x86 DllGetClassObject RVA 0x11a0 / x64 RVA 0x1190.
 * Valid class allocates a factory, QI and Release. On failure
 * the original does not always clear outputs; this model does. */
uint32_t leitz_rp_get_class_object(const leitz_rp_guid *clsid,
    const leitz_rp_guid *iid, int factory_allocated,
    leitz_rp_interface *selected);
/* Original x86 CreateInstance RVA 0x10d0 / x64 RVA 0x10a0. */
uint32_t leitz_rp_create_instance(int outer_unknown_nonnull,
    const leitz_rp_guid *iid, int raster_object_allocated,
    leitz_rp_interface *selected);

/* Explicit original LockServer counter, x86 RVA 0x1140 / x64 0x1140.
 * This counter is NOT either global read by DllCanUnloadNow in this
 * vendor binary, so no canonical COM lifetime relation is invented. */
uint32_t leitz_rp_lock_server(atomic_int *lock_counter, int lock);
/* DLL export x86 RVA 0x1180 / x64 RVA 0x1170; both original globals
 * must be zero. Exact counter lifetime is outside this isolated model. */
uint32_t leitz_rp_can_unload_now(const atomic_int *native_flag_a,
    const atomic_int *native_flag_b);
#endif
