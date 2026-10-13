#ifndef CLASSIC_READ_PAGE_H
#define CLASSIC_READ_PAGE_H
#include <stdint.h>
#include <stddef.h>

/* Manufacturer x86_64 raster2printer _ReadPage:
 * 0x100008d20..0x10000918b. Only the observed raster-input core.
 * This interface is newly authored for offline reconstruction; it is
 * NOT the original compiled function's argument structure or ABI. */
typedef struct {
  uint32_t cups_width;
  uint32_t cups_height;
  uint32_t cups_bytes_per_line;
  uint32_t cups_bits_per_pixel;
  uint32_t cups_color_space;
} classic_page_header;

typedef struct {
  void *context;
  unsigned (*read_header)(void *,classic_page_header *);
  unsigned (*read_pixels)(void *,void *,unsigned);
} classic_page_source;

typedef struct {
  classic_page_header header;
  uint32_t requested_bytes;
  unsigned received_bytes;
  unsigned page_number;
  void *pixels;
} classic_page_result;

/* Return 0: no page header (EOF), 1: a nonzero pixel read,
 * -1: allocation failure or a zero pixel read.
 * The latter results are analysis-level return values, not the OEM ABI.
 * A nonzero short read is accepted, as in the observed manufacturer branch.
 * A successful read header increments the page counter immediately.
 * On a zero read, the allocation is intentionally still returned via
 * result->pixels because cleanup/error unwinding is not yet recovered.
 * The caller owns that allocation on both 1 and -1.
 */
int classic_read_page_core(const classic_page_source *source,
                           unsigned *page_counter,
                           classic_page_result *result);
#endif
