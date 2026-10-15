#ifndef ORIGINAL_RASTER_NATIVE_IO_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_IO_ENTRY_H
#include "original_read_page.h"
#include "original_device_uri.h"

/* OEM macOS 5.1.465 raster2printer Mach-O:
 * _ReadPage at 0x100008d2b..0x100008d3b saves five successive
 * register arguments RDI/RSI/RDX/RCX/R8:
 *   full descriptor*, cups_page_header2_t*, pixels**,
 *   requested_bytes*, page_counter*.
 * OEM _ReadPage at 0x100009180 returns the 32-bit int from rbp-0x2c.
 * It must NOT replace the normal _raster2printer inline page reader.
 *
 * _ResolveDeviceURI at 0x100009a49 accepts the same original FULL
 * descriptor pointer as _CheckPrinterStatus (original 0x10000922c).
 * Its native implementation writes scheme +0x50, hostname +0x450,
 * resource +0x850, username +0xc50 and port +0x1098, ending inside
 * the original 0x10a8-byte descriptor. At 0x100009fa5..0x100009fd3,
 * it returns an unmasked 32-bit integer, not a bool-sized status.
 *
 * The sparse URI structure is deliberately PRIVATE to the existing
 * original-derived resolver algorithm. This header is the common
 * native ABI for the two externally named manufacturer functions.
 */
int ReadPage(classic_oem_read_descriptor *descriptor,
             cups_page_header2_t *header,unsigned char **pixels,
             int *requested_bytes,int *page_counter);
int ResolveDeviceURI(classic_oem_read_descriptor *descriptor);
#endif
