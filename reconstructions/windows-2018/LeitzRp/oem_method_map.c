#include "oem_method_map.h"
#include <string.h>

static const leitz_rp_method_entry original_vtable[] = {
#include "oem_method_map_entries.inc"
};

const leitz_rp_method_entry *leitz_rp_oem_method_at(size_t slot)
{
    return slot < LEITZ_RP_OEM_METHOD_COUNT ? &original_vtable[slot] : NULL;
}

int leitz_rp_oem_direct_return(size_t slot, uint32_t *value)
{
    const leitz_rp_method_entry *method = leitz_rp_oem_method_at(slot);
    if (!method || !value) return -1;
    switch (method->kind) {
    case LEITZ_RP_METHOD_NOTIMPL:
        *value = LEITZ_RP_OEM_E_NOTIMPL;
        return 0;
    case LEITZ_RP_METHOD_S_OK:
        *value = 0;
        return 0;
    case LEITZ_RP_METHOD_ONE:
        *value = 1;
        return 0;
    default:
        return -1;
    }
}

int leitz_rp_oem_filtergraphics_comparison(const char *method_name)
{
    if (!method_name) return -1;
    return strcmp(method_name, "FilterGraphics") != 0;
}
