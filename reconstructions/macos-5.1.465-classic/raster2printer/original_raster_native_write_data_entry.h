#ifndef ORIGINAL_RASTER_NATIVE_WRITE_DATA_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_WRITE_DATA_ENTRY_H
#include "original_read_page.h"
#include <stddef.h>
#include <stdint.h>

/* Raster OEM _WriteData 0x100001450: bytes in %rdi, length in %rsi,
 * socket descriptor in %edx, followed by a full 0x10a8-byte
 * descriptor passed BY VALUE via the native x86_64 stack.
 * Manufacturer proof at the _CheckPrinterStatus caller:
 *   0x100009759   movl  __ZL4sock(%rip),%edx
 *   0x100009762   movl  $0x215,%eax
 *   0x100009781   rep movsq (0x215 * 8 = 0x10a8 bytes)
 *   0x100009792   callq _WriteData
 * This does NOT make _CheckPrinterStatus itself by-value:
 * its entry takes a pointer (0x1000091a9, separately verified).
 */
int WriteData(const uint8_t *bytes,size_t length,int socket_fd,
              classic_oem_read_descriptor descriptor);
#endif
