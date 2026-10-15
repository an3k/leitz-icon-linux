/* Compile-only original-pointer-ABI check against native macOS CUPS.
 * The OEM 0x1000091a9 saves %rdi as a DESCRIPTOR POINTER, while the
 * full descriptor structure accessed by that pointer is 0x10a8 bytes.
 * No hardware or status function executes during compilation. */
#include "original_raster_native_status_entry.h"
#include <stddef.h>
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8u,
               "OEM CheckPrinterStatus referenced descriptor size");
_Static_assert(offsetof(classic_oem_read_descriptor,
                        cups_page_size_name)==0x1050u,
               "OEM descriptor terminal field");
typedef _Bool (*original_status_by_pointer)(classic_oem_read_descriptor *);
_Static_assert(_Generic(&CheckPrinterStatus,
               _Bool (*)(classic_oem_read_descriptor *):1,default:0),
               "OEM status entry must return normalized single-bit boolean");
static original_status_by_pointer original_entry=CheckPrinterStatus;
_Bool classic_native_status_abi_probe(classic_oem_read_descriptor *reference) {
    return original_entry(reference);
}
