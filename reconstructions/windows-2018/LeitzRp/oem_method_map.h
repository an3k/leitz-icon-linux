#ifndef LEITZ_WIN2018_RP_OEM_METHOD_MAP_H
#define LEITZ_WIN2018_RP_OEM_METHOD_MAP_H
#include <stddef.h>
#include <stdint.h>

/* From both ORIGINAL x86/x64 LeitzRp.dll OEM raster COM vtables.
 * Portable disassembly-derived method metadata, NOT COM ABI/vtables.
 * Native method names/argument layouts require separate proof. */
#define LEITZ_RP_OEM_METHOD_COUNT 26u
#define LEITZ_RP_OEM_E_NOTIMPL UINT32_C(0x80004001)

typedef enum {
    LEITZ_RP_METHOD_NATIVE = 0,      /* Contains code, not yet fully modeled */
    LEITZ_RP_METHOD_S_OK = 1,        /* Original code returns 0 */
    LEITZ_RP_METHOD_ONE = 2,         /* Original code returns 1 */
    LEITZ_RP_METHOD_FILTER_NAME = 3, /* Original compares FilterGraphics */
    LEITZ_RP_METHOD_NOTIMPL = 4      /* Returns E_NOTIMPL directly */
} leitz_rp_method_kind;

typedef struct {
    uint32_t x86_rva;
    uint32_t x64_rva;
    leitz_rp_method_kind kind;
} leitz_rp_method_entry;

const leitz_rp_method_entry *leitz_rp_oem_method_at(size_t slot);

/* Returns 0 for a direct constant-return original vtable slot, storing
 * its DWORD return value. Returns -1 for nonconstant/native slots,
 * invalid indices, or a NULL result pointer. */
int leitz_rp_oem_direct_return(size_t slot, uint32_t *value);

/* Native x86 RVA 0x1e80 / x64 RVA 0x1f00: strcmp against the literal
 * "FilterGraphics", then produces a BYTE/BOOL from (strcmp != 0).
 * Do NOT invert this observed predicate to assume a COM capability.
 * -1 on NULL is a safe deviation from unchecked native pointers. */
int leitz_rp_oem_filtergraphics_comparison(const char *method_name);
#endif
