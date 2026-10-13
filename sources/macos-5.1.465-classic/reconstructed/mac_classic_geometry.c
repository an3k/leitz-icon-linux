#include "mac_classic_geometry.h"
#include <limits.h>
#include <stddef.h>

int mac_classic_packed_stride(const mac_classic_page_info *page,
                              unsigned *packed_bytes_per_line) {
  if (!page || !packed_bytes_per_line || !page->width || !page->height ||
      page->width > USHRT_MAX || page->height > USHRT_MAX ||
      (page->bits_per_pixel != 1 && page->bits_per_pixel != 24) ||
      !page->bytes_per_line) return -1;

  /* _ReadPage 0x100008e64..0x100008ea1 divides CUPS row bytes by its
   * bits-per-pixel and rounds up when a remainder is present. */
  const unsigned bpp = page->bits_per_pixel;
  const unsigned bytes = page->bytes_per_line;
  const unsigned packed = bytes / bpp + (bytes % bpp != 0u);
  if (!packed || packed > 120u) return -1;

  /* Validate input storage without silently discarding padded raster bytes.
   * For RGB24 the recovered _DitherPage consumes complete RGB triples;
   * incomplete triples would access past the row in the vendor original. */
  if (bpp == 1) {
    if (bytes < (page->width / 8u + (page->width % 8u != 0u))) return -1;
  } else {
    if (page->width > UINT_MAX / 3u || bytes < page->width * 3u ||
        bytes % 3u != 0u) return -1;
    /* The dither loop packs every complete RGB triple. Its actual output
     * stride, ceil((bytes/3)/8), equals ceil(bytes/24). */
  }

  *packed_bytes_per_line = packed;
  return 0;
}
