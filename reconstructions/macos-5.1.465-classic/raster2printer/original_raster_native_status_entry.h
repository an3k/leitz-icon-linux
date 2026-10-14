#ifndef ORIGINAL_RASTER_NATIVE_STATUS_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_STATUS_ENTRY_H
#include "original_read_page.h"

/* Exact original x86_64 OEM _CheckPrinterStatus pointer call surface:
 * 0x1000091a9: movq %rdi,-0x1100(%rbp)
 * 0x1000091d7: movq -0x1100(%rbp),%rax
 * 0x1000091de: addq $0x50,%rax (descriptor scheme)
 * 0x10000922c: movq -0x1100(%rbp),%rdi
 * 0x100009233: callq _ResolveDeviceURI
 *
 * The argument is a pointer to a 0x10a8-byte descriptor. Do not
 * substitute a by-value struct: the manufacturer uses a pointer.
 * The status routine is dormant in ordinary OEM raster printing;
 * this signature check does not prove its untested error branches. */
int CheckPrinterStatus(classic_oem_read_descriptor *descriptor);
#endif
