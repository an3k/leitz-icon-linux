#include "mac_classic_raster_diagnostics.h"
#include <stdint.h>
#include <stdlib.h>

int mac_classic_print_page_debug_info(FILE *stream,
    const mac_classic_raster_diagnostic_header *h) {
  if (!stream || !h) return -1;
  if (fprintf(stream,
      "cupsBytesPerLine = %u\n"
      "cupsBitsPerColor = %u\n"
      "cupsBitsPerPixel = %u\n"
      "cupsColorOrder = %u\n"
      "cupsWidth=%u; .cupsHeight = %u; .cupsBorderlessScalingFactor=%2.5f\n",
      h->cups_bytes_per_line, h->cups_bits_per_color, h->cups_bits_per_pixel,
      h->cups_color_order, h->cups_width, h->cups_height,
      (double)h->cups_borderless_scaling_factor) < 0) return -1;
  if (fprintf(stream,
      "HWResolution=[%u, %u]; .ImagingBoundingBox=[%u, %u, %u, %u]; "
      ".Margins=[%u, %u]; .PageSize=[%u, %u]; "
      ".cupsImagingBBox=[%2.5f, %2.5f, %2.5f, %2.5f]; "
      "cupsPageSize=[%2.5f, %2.5f]\n",
      h->hw_resolution[0],h->hw_resolution[1],
      h->imaging_bounding_box[0],h->imaging_bounding_box[1],
      h->imaging_bounding_box[2],h->imaging_bounding_box[3],
      h->margins[0],h->margins[1],h->page_size[0],h->page_size[1],
      (double)h->cups_imaging_bbox[0],(double)h->cups_imaging_bbox[1],
      (double)h->cups_imaging_bbox[2],(double)h->cups_imaging_bbox[3],
      (double)h->cups_page_size[0],(double)h->cups_page_size[1]) < 0) return -1;
  return fprintf(stream, "cupsPageSizeName='%.*s';\n",
                 (int)sizeof h->cups_page_size_name,
                 h->cups_page_size_name) < 0 ? -1 : 0;
}

int mac_classic_test_raster(const mac_classic_raster_diagnostic_source *source,
                           void *context, FILE *log, unsigned *pages_seen) {
  if (!source || !source->read_header || !source->read_pixels ||
      !pages_seen) return -1;
  *pages_seen = 0;
  for (;;) {
    mac_classic_raster_diagnostic_header h = {0};
    const int read = source->read_header(context,&h);
    if (read < 0) return -1;
    if (read == 0) return 0;
    if (!h.cups_bytes_per_line || !h.cups_height) return -1;
    /* OEM _test_raster reads precisely one line per call, even though the
     * active _ReadPage function uses a whole-page request. */
    uint8_t *line = malloc(h.cups_bytes_per_line);
    if (!line) return -1;
    ++*pages_seen;
    if (log && fprintf(log, "PAGE: %u %u\n", *pages_seen,
                       h.num_copies) < 0) { free(line); return -1; }
    int error = 0;
    for (unsigned y = 0; y < h.cups_height; ++y) {
      if (source->read_pixels(context,line,h.cups_bytes_per_line)
          != h.cups_bytes_per_line) { error = 1; break; }
    }
    free(line);
    if (error) return -1;
  }
}
