#ifndef LEITZ_WIN2018_UI_DRIVER_INTERFACE_H
#define LEITZ_WIN2018_UI_DRIVER_INTERFACE_H
#include <stdint.h>
#include <stddef.h>

/* Original LeitzUI.dll x86/x64 OEM UI driver-interface acquisition at
 * RVA 0x1100, and final object release at x86 0x10a0/x64 0x1080.
 * A portable, caller-owned abstraction; NOT an IPrintOemUI vtable/ABI.
 */
#define LEITZ_WIN2018_UI_DRIVER_S_OK UINT32_C(0)
#define LEITZ_WIN2018_UI_DRIVER_E_POINTER UINT32_C(0x80004003)
#define LEITZ_WIN2018_UI_DRIVER_E_FAIL UINT32_C(0x80004005)
#define LEITZ_WIN2018_UI_DRIVER_IID_BYTES 16u

/* Native memory-layout GUID 92b05d50-78bc-11d1-9480-00a0c90640b8.
 * x86 .rdata RVA 0x313c; x64 .rdata RVA 0x3210. */
const uint8_t *leitz_win2018_ui_driver_iid(void);

typedef int32_t (*leitz_win2018_ui_query_driver)(
    void *opaque_query_context, const uint8_t iid[16], void **result);
typedef void (*leitz_win2018_ui_release_driver)(
    void *opaque_query_context, void *driver_interface);

typedef struct {
    void *driver_interface;
} leitz_win2018_ui_driver_binding;

/* If already bound, return S_OK without querying the incoming object.
 * Otherwise QI with the exact OEM IID; success means signed HRESULT >= 0.
 * A failed QI clears the interface slot and maps HRESULT to E_FAIL.
 * Original code does not validate NULL input; the portable model does.
 * Do not infer that an HRESULT of S_OK implies a non-NULL QI output;
 * the original does not check that pointer after successful QI.
 */
uint32_t leitz_win2018_ui_publish_driver_interface(
    leitz_win2018_ui_driver_binding *binding,
    void *opaque_query_context, leitz_win2018_ui_query_driver query);

/* Mirrors original UI destructor: if the saved driver interface exists,
 * call its IUnknown::Release slot and clear the stored pointer.
 * The actual native destructor additionally frees its own object.
 * For a non-NULL interface, a release callback is required and checked
 * before state mutation. This is a deliberate safety adaptation.
 */
uint32_t leitz_win2018_ui_unbind_driver_interface(
    leitz_win2018_ui_driver_binding *binding,
    void *opaque_query_context, leitz_win2018_ui_release_driver release);
#endif
