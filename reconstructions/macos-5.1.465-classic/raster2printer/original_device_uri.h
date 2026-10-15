#ifndef CLASSIC_ORIGINAL_DEVICE_URI_H
#define CLASSIC_ORIGINAL_DEVICE_URI_H
#include "original_read_page.h"

/* The separate sparse URI struct and full raster struct previously
 * described the same OEM memory address. Keep the compatibility name
 * but use one C11 type; native fields are position-asserted in
 * original_read_page.h. */
typedef classic_oem_read_descriptor classic_original_raster_descriptor;

/* Recovered manufacturer _ResolveDeviceURI 0x100009a30..0x100009fd3.
 * This experimental function can open a REAL TCP connection for an ipp URI.
 * Do not invoke with a device URI except in an isolated reference test.
 * 0=URI absent/invalid; 1=parsed (even if socket connect failed).
 */
int classic_original_resolve_device_uri(
    classic_original_raster_descriptor *descriptor);
/* The two file-private C++ globals are independently identified by
 * the authenticated OEM Mach-O _CheckPrinterStatus / _ResolveDeviceURI
 * disassembly at 0x1000091fb, 0x100009285, 0x100009afb and 0x100009d51.
 * On macOS, use their actual OEM Mach-O symbol names. Both recovered
 * functions already include this header and therefore share one instance.
 * These global assembler labels alone do not reproduce all vendor ABI. */
#if defined(__APPLE__)
extern int classic_original_socket __asm__("__ZL4sock");
extern const char *classic_original_resolved_uri
    __asm__("__ZL12resolved_uri");
#else
extern int classic_original_socket;
extern const char *classic_original_resolved_uri;
#endif
#endif
