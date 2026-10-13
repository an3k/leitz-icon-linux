/* Classic macOS 5.1.465; auxiliary raster functions reconstructed from
 * _test_raster (0x100003c60) and _PrintPageDebugInfo (0x100003e50).
 * Neither function has an observed call from the standard print path.
 * This interface is deliberately CUPS-independent; it is not verbatim OEM C.
 */
#ifndef MAC_CLASSIC_RASTER_DIAGNOSTICS_H
#define MAC_CLASSIC_RASTER_DIAGNOSTICS_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  unsigned cups_bytes_per_line, cups_bits_per_color, cups_bits_per_pixel;
  unsigned cups_color_order, cups_width, cups_height;
  unsigned num_copies; /* cups_page_header2_t +0x154: NumCopies */
  float cups_borderless_scaling_factor;
  unsigned hw_resolution[2], imaging_bounding_box[4], margins[2], page_size[2];
  float cups_imaging_bbox[4], cups_page_size[2];
  char cups_page_size_name[64];
} mac_classic_raster_diagnostic_header;

/* Recovered OEM page-header field inventory and stderr messages.
 * The OEM writes a fixed built-file path and source line prefix before each
 * field; this portable renderer intentionally omits build-machine paths. */
int mac_classic_print_page_debug_info(FILE *stream,
    const mac_classic_raster_diagnostic_header *header);

typedef struct {
  /* Header reader: 1=page, 0=EOF, negative=error. */
  int (*read_header)(void *, mac_classic_raster_diagnostic_header *);
  /* Return bytes received, 0 on EOF/error (matches the observed CUPS API). */
  size_t (*read_pixels)(void *, uint8_t *, size_t);
} mac_classic_raster_diagnostic_source;

/* Original _test_raster opens stdin through cupsRasterOpen, then reads one
 * cupsBytesPerLine row at a time for cupsHeight rows on each page.
 * This version receives the open stream as callbacks instead of assuming
 * that CUPS owns descriptor 0. A short row is rejected for safety.
 */
int mac_classic_test_raster(const mac_classic_raster_diagnostic_source *source,
                           void *context, FILE *log, unsigned *pages_seen);
#endif
