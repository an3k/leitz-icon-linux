/* Mach-O classic macOS 5.1.465 _ReadPage 0x100008d20..0x10000918b.
 * CUPS calls isolated as injected functions for portable source validation.
 * This is NOT a full CUPS filter or a replacement for Apple's CUPS SDK.
 */
#ifndef MAC_CLASSIC_READ_PAGE_H
#define MAC_CLASSIC_READ_PAGE_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
  unsigned height;         /* CUPS page header height (rows) */
  unsigned bytes_per_line; /* Original uses CUPS bytes per line for allocation */
  unsigned width;          /* CUPS cupsWidth, populated by the adapter */
  unsigned bits_per_pixel; /* 1 for packed mono, 24 for RGB */
} mac_classic_page_info;

typedef int (*mac_classic_read_header_fn)(void *ctx, mac_classic_page_info *out);
typedef size_t (*mac_classic_read_pixels_fn)(void *ctx, uint8_t *out, size_t bytes);
typedef struct {
  mac_classic_read_header_fn read_header;
  mac_classic_read_pixels_fn read_pixels;
} mac_classic_page_source;

enum { MAC_CLASSIC_PAGE_ERROR=-1, MAC_CLASSIC_PAGE_EOF=0, MAC_CLASSIC_PAGE_OK=1 };

/* Read one full page. On no header: EOF, on bad data: ERROR.
 * Allocate a fresh buffer on success. Caller owns and frees *pixels.
 * The original does a single cupsRasterReadPixels of (height*bpl) and
 * distinguishes zero vs nonzero; this hardened extraction also rejects
 * nonzero short reads, deliberately differing from the vendor executable.
 */
int mac_classic_read_page(const mac_classic_page_source *src, void *context,
                          mac_classic_page_info *header, uint8_t **pixels,
                          size_t *length, unsigned *page_count);
#endif
