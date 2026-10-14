#ifndef ORIGINAL_CLASSIC_READ_PAGE_H
#define ORIGINAL_CLASSIC_READ_PAGE_H
#include <cups/raster.h>
#include <stddef.h>
#include <stdint.h>

/* Original macOS classic raster2printer ReadPage (0x100008d20..0x10000918b).
 * This C descriptor reproduces the original field offsets used by ReadPage,
 * without claiming its unknown member names or the entire original C++ ABI.
 */
typedef struct {
    cups_raster_t *cups_raster;              /* 0x0000 */
    unsigned char unknown_0008[0x1c-8];
    uint16_t cups_height;                   /* 0x001c */
    uint16_t cups_width;                    /* 0x001e */
    uint32_t packed_bytes_per_line;         /* 0x0020 */
    unsigned char unknown_0024[0x30-0x24];
    uint32_t cups_bits_per_pixel;           /* 0x0030 */
    uint32_t cups_bytes_per_line;           /* 0x0034 */
    unsigned char unknown_0038[0x3c-0x38];
    uint32_t cups_color_space;              /* 0x003c */
    unsigned char unknown_0040[0x1050-0x40];
    char cups_page_size_name[0x58];         /* 0x1050 */
} classic_oem_read_descriptor;

_Static_assert(offsetof(classic_oem_read_descriptor,cups_height)==0x1c,"OEM height offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_width)==0x1e,"OEM width offset");
_Static_assert(offsetof(classic_oem_read_descriptor,packed_bytes_per_line)==0x20,"OEM raster-stride offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_bits_per_pixel)==0x30,"OEM bpp offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_bytes_per_line)==0x34,"OEM line width offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_color_space)==0x3c,"OEM color-space offset");
_Static_assert(offsetof(classic_oem_read_descriptor,cups_page_size_name)==0x1050,"OEM media name offset");
_Static_assert(sizeof(classic_oem_read_descriptor)==0x10a8,"OEM descriptor size");

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
