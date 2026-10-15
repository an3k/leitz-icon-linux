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
 * OEM epilogue 0x1000099f2..0x100009a1d loads an 8-bit status,
 * masks AL with 1 and zero-extends it to EAX before returning.
 * This establishes the externally observable bool-sized 0/1 return.
 * The original C++ source spelling (bool vs another one-byte type)
 * is not recoverable verbatim from machine code alone.
 *
 * The status routine is dormant in ordinary OEM raster printing;
 * this signature check does not prove its untested error branches. */
_Bool CheckPrinterStatus(classic_oem_read_descriptor *descriptor);
#endif
