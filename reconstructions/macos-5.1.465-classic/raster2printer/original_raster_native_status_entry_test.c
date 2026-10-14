/* Compile-only source-level native ABI contract. The original
 * by-value stack argument is 0x10a8 bytes with scheme at +0x50.
 * No status or network callbacks execute during this check. */
#include "original_raster_native_status_entry.h"
#include <stddef.h>
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8u,
               "OEM CheckPrinterStatus stack copy size");
_Static_assert(offsetof(classic_oem_read_descriptor,
                        cups_page_size_name)==0x1050u,
               "OEM descriptor terminal field");
typedef int (*original_status_by_value)(classic_oem_read_descriptor);
static original_status_by_value original_entry=CheckPrinterStatus;
int classic_native_status_abi_probe(classic_oem_read_descriptor copy) {
    return original_entry(copy);
}
