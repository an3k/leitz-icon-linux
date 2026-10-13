/* Reconstructed from _ReadPage in classic raster2printer Mach-O, addresses:
 * 0x100008d46: cupsRasterReadHeader2 -> EOF if zero
 * 0x100008d5f: page index increment
 * 0x10000903a..0x100009051: multiply two 32-bit page header dimensions
 * 0x10000905e: calloc(1,product)
 * 0x10000906a..0x100009083: cupsRasterReadPixels once for all page bytes
 * 0x100009086..0x1000090c0: zero-length read -> page read failure.
 * External CUPS integration is provided by callbacks; no macOS SDK needed.
 */
#include "mac_classic_read_page.h"
#include <stdint.h>
#include <stdlib.h>

int mac_classic_read_page(const mac_classic_page_source *src, void *context,
                          mac_classic_page_info *header, uint8_t **pixels,
                          size_t *length, unsigned *page_count) {
  if (!src || !src->read_header || !src->read_pixels || !header ||
      !pixels || !length || !page_count) return MAC_CLASSIC_PAGE_ERROR;
  *pixels = NULL;
  *length = 0;
  const int header_result = src->read_header(context, header);
  if (header_result == 0) return MAC_CLASSIC_PAGE_EOF;
  if (header_result < 0) return MAC_CLASSIC_PAGE_ERROR;
  if (!header->height || !header->bytes_per_line ||
      (size_t)header->height > SIZE_MAX / (size_t)header->bytes_per_line)
    return MAC_CLASSIC_PAGE_ERROR;
  size_t bytes = (size_t)header->height * header->bytes_per_line;
  uint8_t *data = (uint8_t *)calloc(1,bytes);
  if (!data) return MAC_CLASSIC_PAGE_ERROR;
  const size_t got = src->read_pixels(context,data,bytes);
  if (got != bytes) { free(data); return MAC_CLASSIC_PAGE_ERROR; }
  *pixels = data;
  *length = bytes;
  ++*page_count;
  return MAC_CLASSIC_PAGE_OK;
}
