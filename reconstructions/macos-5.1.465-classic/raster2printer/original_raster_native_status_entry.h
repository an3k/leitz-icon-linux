#ifndef ORIGINAL_RASTER_NATIVE_STATUS_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_STATUS_ENTRY_H
#include "original_read_page.h"

/* Manufacturer macOS 5.1.465 _CheckPrinterStatus (0x100009190):
 * the original stack-param descriptor is a full 0x10a8-byte copy,
 * not a pointer-to-descriptor parameter. This definition recovers
 * the native x86_64 C by-value call surface and delegates the body's
 * already recovered status/error logic without duplicating it.
 *
 * Important: the original compiler's complete C++ ABI and undefined
 * socket/error branches remain unverified. Ordinary OEM raster jobs do
 * not call this dormant function. */
int CheckPrinterStatus(classic_oem_read_descriptor descriptor);
#endif
