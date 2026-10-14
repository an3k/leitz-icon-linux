#ifndef CLASSIC_ORIGINAL_DEVICE_URI_H
#define CLASSIC_ORIGINAL_DEVICE_URI_H
#include <stddef.h>
#include <stdint.h>

/* Actual original raster descriptor byte offsets established by direct
 * Mach-O _ResolveDeviceURI disassembly, 0x100009ba5..0x100009c4a.
 * The intervening bytes are not assigned invented interpretations. */
typedef struct {
    uint8_t unknown_prefix[0x50];
    char scheme[1024];
    char hostname[1024];
    char resource[1024];
    char username[1024];
    uint8_t unknown_to_port[0x48];
    int port;
} classic_original_raster_descriptor;
_Static_assert(offsetof(classic_original_raster_descriptor,scheme)==0x50,
               "original scheme offset mismatch");
_Static_assert(offsetof(classic_original_raster_descriptor,hostname)==0x450,
               "original hostname offset mismatch");
_Static_assert(offsetof(classic_original_raster_descriptor,resource)==0x850,
               "original resource offset mismatch");
_Static_assert(offsetof(classic_original_raster_descriptor,username)==0xc50,
               "original username offset mismatch");
_Static_assert(offsetof(classic_original_raster_descriptor,port)==0x1098,
               "original port offset mismatch");

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
