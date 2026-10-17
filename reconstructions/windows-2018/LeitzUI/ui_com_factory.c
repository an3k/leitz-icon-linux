#include "ui_com_factory.h"
#include <string.h>

static const leitz_win2018_ui_guid original_guids[] = {
    /* CLSID from x64 RVA 0x3220 and x86 RVA 0x314c:
       abce80d7-9f46-11d1-882a-00c04fb961ec */
    {{0xd7,0x80,0xce,0xab,0x46,0x9f,0xd1,0x11,
      0x88,0x2a,0x00,0xc0,0x4f,0xb9,0x61,0xec}},
    /* IID_IUnknown */
    {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}},
    /* IID_IClassFactory */
    {{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}},
    /* UI QueryInterface's additional IID at x64 RVA 0x3230:
       c6a7a9d0-774c-11d1-947f-00a0c90640b8 */
    {{0xd0,0xa9,0xa7,0xc6,0x4c,0x77,0xd1,0x11,
      0x94,0x7f,0x00,0xa0,0xc9,0x06,0x40,0xb8}}
};

const leitz_win2018_ui_guid *leitz_win2018_ui_known_guid(
    leitz_win2018_ui_guid_kind kind)
{
    return kind >= LEITZ_UI_GUID_CLSID &&
           kind <= LEITZ_UI_GUID_IPRINTOEMUI ? &original_guids[kind] : NULL;
}

static int matches(const leitz_win2018_ui_guid *lhs,
                   leitz_win2018_ui_guid_kind rhs)
{
    return lhs && memcmp(lhs, &original_guids[rhs], sizeof *lhs) == 0;
}

uint32_t leitz_win2018_ui_query_interface(
    leitz_win2018_ui_interface current,
    const leitz_win2018_ui_guid *iid,
    leitz_win2018_ui_interface *out_interface)
{
    if (!out_interface) return LEITZ_UI_E_POINTER;
    *out_interface = LEITZ_UI_INTERFACE_NONE;
    if (!iid) return LEITZ_UI_E_POINTER; /* OEM dereferences unchecked. */
    if (current == LEITZ_UI_INTERFACE_FACTORY &&
        (matches(iid, LEITZ_UI_GUID_IUNKNOWN) ||
         matches(iid, LEITZ_UI_GUID_ICLASSFACTORY))) {
        *out_interface = current;
        return LEITZ_UI_S_OK;
    }
    if (current == LEITZ_UI_INTERFACE_PRINTER &&
        (matches(iid, LEITZ_UI_GUID_IUNKNOWN) ||
         matches(iid, LEITZ_UI_GUID_IPRINTOEMUI))) {
        *out_interface = current;
        return LEITZ_UI_S_OK;
    }
    return LEITZ_UI_E_NOINTERFACE;
}

uint32_t leitz_win2018_ui_get_class_object(
    const leitz_win2018_ui_guid *clsid,
    const leitz_win2018_ui_guid *iid,
    int original_allocation_successful,
    leitz_win2018_ui_interface *out_interface)
{
    if (!out_interface) return LEITZ_UI_E_POINTER;
    *out_interface = LEITZ_UI_INTERFACE_NONE;
    if (!clsid || !iid) return LEITZ_UI_E_POINTER;
    if (!matches(clsid, LEITZ_UI_GUID_CLSID))
        return LEITZ_UI_CLASS_E_CLASSNOTAVAILABLE;
    if (!original_allocation_successful) return LEITZ_UI_E_OUTOFMEMORY;
    /* Native factory constructs object with reference count 1, QI,
     * then releases the original reference, retaining only the QI ref. */
    return leitz_win2018_ui_query_interface(
        LEITZ_UI_INTERFACE_FACTORY, iid, out_interface);
}

uint32_t leitz_win2018_ui_create_instance(
    int outer_unknown_is_nonnull,
    const leitz_win2018_ui_guid *iid,
    int original_allocation_successful,
    leitz_win2018_ui_interface *out_interface)
{
    if (!out_interface) return LEITZ_UI_E_POINTER;
    *out_interface = LEITZ_UI_INTERFACE_NONE;
    if (outer_unknown_is_nonnull) return LEITZ_UI_CLASS_E_NOAGGREGATION;
    if (!iid) return LEITZ_UI_E_POINTER;
    if (!original_allocation_successful) return LEITZ_UI_E_OUTOFMEMORY;
    /* Native CreateInstance similarly QIs and releases the new object. */
    return leitz_win2018_ui_query_interface(
        LEITZ_UI_INTERFACE_PRINTER, iid, out_interface);
}

uint32_t leitz_win2018_ui_lock_server(atomic_int *lock_count, int lock)
{
    if (!lock_count) return LEITZ_UI_E_POINTER;
    if (lock) (void)atomic_fetch_add(lock_count, 1);
    else (void)atomic_fetch_sub(lock_count, 1);
    return LEITZ_UI_S_OK;
}

uint32_t leitz_win2018_ui_can_unload_now(
    const atomic_int *active_objects, const atomic_int *server_locks)
{
    if (!active_objects || !server_locks) return LEITZ_UI_S_FALSE;
    return atomic_load(active_objects) == 0 &&
           atomic_load(server_locks) == 0 ? LEITZ_UI_S_OK : LEITZ_UI_S_FALSE;
}
