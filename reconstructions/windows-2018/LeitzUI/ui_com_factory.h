#ifndef LEITZ_WIN2018_UI_COM_FACTORY_H
#define LEITZ_WIN2018_UI_COM_FACTORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

/* Exact 16-byte Windows GUID memory order. Original Windows 2018
 * LeitzUI.dll x86/x64 DllGetClassObject, IClassFactory::CreateInstance,
 * QueryInterface and LockServer. New portable API, NOT original COM ABI.
 */
#define LEITZ_UI_S_OK UINT32_C(0x00000000)
#define LEITZ_UI_S_FALSE UINT32_C(0x00000001)
#define LEITZ_UI_E_POINTER UINT32_C(0x80004003)
#define LEITZ_UI_E_NOINTERFACE UINT32_C(0x80004002)
#define LEITZ_UI_E_OUTOFMEMORY UINT32_C(0x8007000E)
#define LEITZ_UI_CLASS_E_NOAGGREGATION UINT32_C(0x80040110)
#define LEITZ_UI_CLASS_E_CLASSNOTAVAILABLE UINT32_C(0x80040111)

typedef struct { uint8_t bytes[16]; } leitz_win2018_ui_guid;
typedef enum {
    LEITZ_UI_INTERFACE_NONE = 0,
    LEITZ_UI_INTERFACE_FACTORY = 1,
    LEITZ_UI_INTERFACE_PRINTER = 2
} leitz_win2018_ui_interface;

typedef enum {
    LEITZ_UI_GUID_CLSID = 0,
    LEITZ_UI_GUID_IUNKNOWN,
    LEITZ_UI_GUID_ICLASSFACTORY,
    LEITZ_UI_GUID_IPRINTOEMUI
} leitz_win2018_ui_guid_kind;

/* The original GUIDs are in native Windows in-memory byte order. */
const leitz_win2018_ui_guid *leitz_win2018_ui_known_guid(
    leitz_win2018_ui_guid_kind kind);

/* Returns E_NOINTERFACE for unsupported interfaces; outputs NONE first.
 * Original x64 factory QueryInterface RVA 0x1280, UI QI RVA 0x1010.
 * x86 equivalents RVA 0x12e0 and RVA 0x1010. */
uint32_t leitz_win2018_ui_query_interface(
    leitz_win2018_ui_interface current,
    const leitz_win2018_ui_guid *iid,
    leitz_win2018_ui_interface *out_interface);

/* Original DllGetClassObject x64 RVA 0x1400, x86 RVA 0x1460.
 * original_allocation_successful emulates the single allocation
 * decision, not the original heap or factory object's lifetime.
 */
uint32_t leitz_win2018_ui_get_class_object(
    const leitz_win2018_ui_guid *clsid,
    const leitz_win2018_ui_guid *iid,
    int original_allocation_successful,
    leitz_win2018_ui_interface *out_interface);

/* Original IClassFactory::CreateInstance x64 RVA 0x1310, x86 RVA 0x1390.
 * An original aggregation pointer, if non-NULL, is rejected.
 */
uint32_t leitz_win2018_ui_create_instance(
    int outer_unknown_is_nonnull,
    const leitz_win2018_ui_guid *iid,
    int original_allocation_successful,
    leitz_win2018_ui_interface *out_interface);

/* LockServer x64 RVA 0x13c0, x86 RVA 0x1410: atomic inc/dec.
 * DllCanUnloadNow x64 RVA 0x13e0, x86 RVA 0x1440: returns S_OK
 * iff BOTH separately maintained native counters are zero.
 * Counters are supplied; this does not infer their native lifetimes.
 */
uint32_t leitz_win2018_ui_lock_server(atomic_int *lock_count, int lock);
uint32_t leitz_win2018_ui_can_unload_now(
    const atomic_int *active_objects, const atomic_int *server_locks);
#endif
