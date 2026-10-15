#ifndef ORIGINAL_CLASSIC_READ_PAGE_H
#define ORIGINAL_CLASSIC_READ_PAGE_H
#include <cups/raster.h>
#include <stddef.h>
#include <stdint.h>

/* Original macOS classic raster2printer ReadPage (0x100008d20..0x10000918b).
 * This C descriptor reproduces the original field offsets used by ReadPage,
 * without claiming its unknown member names or the entire original C++ ABI.
 */
/* Canonical native byte layout shared by _ReadPage, _ResolveDeviceURI,
 * _CheckPrinterStatus and the raster job. Each named field position is
 * independently supported by verified OEM original instructions.
 * Unknown bytes remain opaque; no original C++ spelling is asserted.
 * The actual CUPS page header's cupsPageSizeName has 64 bytes.
 * The OEM copies its string into the descriptor at +0x1050; the
 * manufacturer URI parser stores the port at +0x1098, separated by
 * eight currently opaque bytes at +0x1090..+0x1097. Mode 3 uses
 * +0x24 to retain its last width between pages: OEM 0x100007c78
 * loads this 32-bit field, and 0x100007cb0 writes a zero-extended
 * width BYTE before sending ESC D (confirmed by original Mach-O
 * SHA-256 audit 38778100810). This source-level name is inferred,
 * not claimed to be the OEM C++ declaration. Do NOT invent
 * a union/overlap between these independent observed locations.
 */
typedef struct {
    cups_raster_t *cups_raster;               /* +0x0000 */
    uint32_t print_through_prn;               /* +0x0008 */
    unsigned char unknown_000c[4];
    uint32_t cut_every_label;                 /* +0x0010 */
    uint32_t high_resolution;                 /* +0x0014 */
    uint32_t debug_mode;                      /* +0x0018 */
    uint16_t cups_height;                     /* +0x001c */
    uint16_t cups_width;                      /* +0x001e */
    uint32_t packed_bytes_per_line;           /* +0x0020 */
    uint32_t mode3_previous_width;             /* +0x0024, OEM 0x7c78 / 0x7cb0 */
    unsigned char unknown_0028[0x30-0x28];
    uint32_t cups_bits_per_pixel;             /* +0x0030 */
    uint32_t cups_bytes_per_line;             /* +0x0034 */
    unsigned char unknown_0038[0x3c-0x38];
    uint32_t cups_color_space;                /* +0x003c */
    uint32_t optimization_level;              /* +0x0040 */
    unsigned char unknown_0044[0x4c-0x44];
    uint32_t print_quality;                   /* +0x004c */
    char scheme[1024];                        /* +0x0050 */
    char hostname[1024];                      /* +0x0450 */
    char resource[1024];                      /* +0x0850 */
    char username[1024];                      /* +0x0c50 */
    char cups_page_size_name[64];             /* +0x1050..0x108f */
    unsigned char unknown_1090[8];            /* +0x1090..0x1097 */
    int port;                                  /* +0x1098..0x109b */
    unsigned char unknown_109c[0x0c];        /* +0x109c..0x10a7 */
} classic_oem_read_descriptor;

_Static_assert(offsetof(classic_oem_read_descriptor,print_through_prn)==0x08,"OEM PRN offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cut_every_label)==0x10,"OEM cutter offset");
_Static_assert(offsetof(classic_oem_read_descriptor,high_resolution)==0x14,"OEM resolution offset");
_Static_assert(offsetof(classic_oem_read_descriptor,debug_mode)==0x18,"OEM debug offset");
_Static_assert(offsetof(classic_oem_read_descriptor,optimization_level)==0x40,"OEM optimization offset");
_Static_assert(offsetof(classic_oem_read_descriptor,print_quality)==0x4c,"OEM quality offset");
_Static_assert(offsetof(classic_oem_read_descriptor,scheme)==0x50,"OEM scheme offset");
_Static_assert(offsetof(classic_oem_read_descriptor,hostname)==0x450,"OEM hostname offset");
_Static_assert(offsetof(classic_oem_read_descriptor,resource)==0x850,"OEM resource offset");
_Static_assert(offsetof(classic_oem_read_descriptor,username)==0xc50,"OEM username offset");
_Static_assert(offsetof(classic_oem_read_descriptor,unknown_1090)==0x1090,"OEM opaque slot offset");
_Static_assert(offsetof(classic_oem_read_descriptor,port)==0x1098,"OEM URI port offset");
_Static_assert(offsetof(classic_oem_read_descriptor,unknown_109c)==0x109c,"OEM tail offset");
_Static_assert(sizeof(((classic_oem_read_descriptor *)0)->cups_page_size_name)==64,"CUPS page-name capacity");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_height)==0x1c,"OEM height offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_width)==0x1e,"OEM width offset");
_Static_assert(offsetof(classic_oem_read_descriptor,packed_bytes_per_line)==0x20,"OEM raster-stride offset");
_Static_assert(offsetof(classic_oem_read_descriptor,mode3_previous_width)==0x24,"OEM mode-3 cached-width offset");
_Static_assert(offsetof(classic_oem_read_descriptor,unknown_0028)==0x28,"OEM still-opaque descriptor offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_bits_per_pixel)==0x30,"OEM bpp offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_bytes_per_line)==0x34,"OEM line width offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_color_space)==0x3c,"OEM color-space offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_page_size_name)==0x1050,"OEM media name offset");
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8,"OEM descriptor size");

/* Manufacturer _ReadPage 0x100008e64..0x100008e9f:
 * unsigned divl computes the initial quotient, then *signed* idivl
 * independently tests whether the 32-bit remainder is nonzero.
 * For ordinary CUPS lengths <= INT32_MAX these produce conventional
 * ceiling division; for larger values the signed/unsigned distinction
 * is observable and must not be simplified away in the reconstruction.
 * A zero divisor and the processor's trapping signed-overflow input
 * remain undefined in the OEM and are deliberately not handled here. */
static inline uint32_t classic_oem_read_page_packed_stride(
        uint32_t bytes_per_line,uint32_t bits_per_pixel) {
    return bytes_per_line / bits_per_pixel +
        (((int32_t)bytes_per_line % (int32_t)bits_per_pixel)!=0);
}

/* OEM return: 0 on no header or zero pixels; 1 on any nonzero pixel count.
 * Original error handling deliberately leaves *pixels pointing to freed
 * storage after the zero-pixels case. Callers must not dereference or free
 * that dangling pointer. This is a faithful recovery, NOT a safe API.
 */
int classic_original_read_page(classic_oem_read_descriptor *descriptor,
                               cups_page_header2_t *page_header,
                               unsigned char **pixels,
                               int *requested_bytes,
                               int *page_counter);
#endif
