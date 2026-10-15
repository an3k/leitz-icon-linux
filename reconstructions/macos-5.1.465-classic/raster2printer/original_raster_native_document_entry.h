#ifndef ORIGINAL_RASTER_NATIVE_DOCUMENT_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_DOCUMENT_ENTRY_H
#include "original_read_page.h"
#include <stddef.h>

/* Authenticated macOS 5.1.465 raster2printer call ABI:
 * Original caller copies 0x10a8 bytes (0x215 qwords) onto the stack
 * immediately before _StartDocument at 0x100005463..0x10000548e,
 * and _EndDocument at 0x1000062eb..0x10000632d.
 * Both OEM callee entries inspect the first stack argument at rbp+0x10:
 * _StartDocument 0x10000689b, _EndDocument 0x100008acb.
 * In EndDocument, clearing descriptor+0 only changes this local copy.
 * The separate project callback ABI is not attributed to the OEM. */
void StartDocument(classic_oem_read_descriptor descriptor);
void EndDocument(classic_oem_read_descriptor descriptor);
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8,
               "original by-value document descriptor size");
#endif
