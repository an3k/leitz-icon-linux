#ifndef LEITZ_WIN2018_RP_OEM_HOST_H
#define LEITZ_WIN2018_RP_OEM_HOST_H
#include <stddef.h>
#include <stdint.h>

/* Original LeitzRp.dll 2018 x86/x64 OEM plugin entry decisions.
 * Portable caller-owned callbacks; NOT native IPrintOemUni2/Windows COM ABI. */
#define LEITZ_RP_S_OK UINT32_C(0)
#define LEITZ_RP_E_FAIL UINT32_C(0x80004005)
#define LEITZ_RP_E_POINTER UINT32_C(0x80004003)
#define LEITZ_RP_DRIVER_IID_SIZE 16u

typedef void (*leitz_rp_set_last_error)(void *context, uint32_t error_code);

/* LeitzRp.dll GetInfo: x86 RVA 0x1ed0; x64 RVA 0x1f20.
 * Modes 1 and 3: require a DWORD output and write DIPL or 1.
 * Mode 4: write *required=8, return E_FAIL with NO SetLastError.
 * All other modes: E_FAIL, SetLastError(87), no required-size change.
 * For 1/3 without an output or capacity<4: required=4, E_FAIL,
 * SetLastError(122). All other scalar modes are rejected safely.
 */
uint32_t leitz_rp_oem_get_info(uint32_t mode, uint8_t *output,
                              size_t capacity, uint32_t *required,
                              leitz_rp_set_last_error error_cb,
                              void *context);

/* GUID d67ebbf1-78bf-11d1-9480-00a0c90640b8 (Windows byte order)
 * recovered from original x86 RVA 0x5074 and x64 RVA 0x5268. */
const uint8_t *leitz_rp_oem_driver_iid(void);

typedef int32_t (*leitz_rp_host_query_fn)(void *context,
                              const uint8_t iid[LEITZ_RP_DRIVER_IID_SIZE],
                              void **result);
typedef void (*leitz_rp_release_interface)(void *context, void *interface);
typedef struct {
    void *driver_interface;
} leitz_rp_oem_binding;

/* LeitzRp OEM PublishDriverInterface: x86 RVA 0x1f80 and
 * x64 RVA 0x1ff0. Existing non-NULL pointer skips QueryInterface.
 * Negative HRESULT always maps to E_FAIL and clears the slot.
 * Nonnegative HRESULT is S_OK, even if the returned pointer is NULL.
 * Original instance member resides at +0x08 (x86), +0x10 (x64).
 * A NULL callback or state is rejected safely instead of native UB.
 */
uint32_t leitz_rp_oem_publish_driver(leitz_rp_oem_binding *binding,
                                   leitz_rp_host_query_fn query,
                                   void *context);
/* Original destruction/reset uses IUnknown::Release on cached interface;
 * this portable cleanup models that release, not native object lifetime. */
uint32_t leitz_rp_oem_release_driver(leitz_rp_oem_binding *binding,
                                   leitz_rp_release_interface release,
                                   void *context);
#endif
