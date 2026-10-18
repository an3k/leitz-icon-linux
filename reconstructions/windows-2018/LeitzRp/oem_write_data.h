#ifndef LEITZ_WIN2018_RP_OEM_WRITE_DATA_H
#define LEITZ_WIN2018_RP_OEM_WRITE_DATA_H
#include <stddef.h>
#include <stdint.h>

/* Portable reconstruction of the original Windows LeitzRp.dll WriteData
 * helper, x86 RVA 0x29f0 / x64 RVA 0x29e0. Not the native COM ABI.
 * Native IPrintOemDriverUni vtable slots: x86 +0x10/+0x28,
 * x64 +0x20/+0x50. The second slot is called with literal 200.
 */
#define LEITZ_RP_WRITE_S_OK UINT32_C(0)
#define LEITZ_RP_WRITE_E_FAIL UINT32_C(0x80004005)
#define LEITZ_RP_WRITE_ERROR_CODE UINT32_C(200)

typedef enum {
    LEITZ_RP_WRITE_X86 = 32,
    LEITZ_RP_WRITE_X64 = 64
} leitz_rp_write_arch;

typedef int32_t (*leitz_rp_oem_spool_write)(void *context, void *device,
    const uint8_t *bytes, uint32_t requested, uint32_t *written);
typedef int32_t (*leitz_rp_oem_write_error)(void *context, void *device,
    const uint8_t *bytes, uint32_t requested, uint32_t error_code);

typedef struct {
    leitz_rp_write_arch architecture;
    uint64_t accumulated_bytes; /* Original x86 DWORD +0x0c; x64 QWORD +0x18. */
    void *device;
    void *callback_context;
    leitz_rp_oem_spool_write write;
    leitz_rp_oem_write_error report;
} leitz_rp_oem_write_state;

/* Increment accumulated_bytes BEFORE invoking the host writer, with native
 * width/wrap behavior even if host reports failure. The success condition is
 * (written == requested && hresult != E_FAIL), NOT SUCCEEDED(hresult).
 * Otherwise invoke report with code 200 and return E_FAIL unconditionally.
 * The manufacturer's helper does not validate pointer sizes; this bounded
 * adapter refuses invalid inputs before modifying state. length 0 is valid
 * and still invokes the host, matching the original control flow.
 */
int32_t leitz_rp_oem_write_data(leitz_rp_oem_write_state *state,
    const uint8_t *bytes, uint32_t length);

/* Adapter for the existing leitz_win2018_row_write callback signature;
 * no duplicate raster encoding or automatic job lifecycle is imposed. */
int32_t leitz_rp_oem_write_callback(void *context,
    const uint8_t *bytes, size_t length);
#endif
