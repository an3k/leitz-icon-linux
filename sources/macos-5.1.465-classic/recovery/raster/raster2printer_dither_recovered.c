#include "raster2printer_dither_recovered.h"
#include "raster2printer_quality_recovered.h"
#include <stddef.h>

/* This is only the per-pixel computation from OEM _DitherPage, not the
 * original filter's allocation, CUPS handling, debug or error paths.
 * Values and scaling constants are in raster2printer_quality_recovered.c.
 * No safety bounds, gamma conversion, alternative algorithm or new
 * threshold adjustment is added. The arithmetic follows the previously
 * documented Mach-O _DitherPage inner loop. Exact binary parity remains
 * unverified until the original executable can be run on macOS. */
void classic_raster_dither_rgb24_recovered(unsigned quality,
              const uint8_t *rgb, unsigned height,
              unsigned bytes_per_line, uint8_t *monochrome) {
  const classic_raster_halftone_table *table =
      &classic_raster_quality_tables[quality];
  const unsigned pixels_per_line = bytes_per_line / 3u;
  const unsigned packed_bytes_per_line = (pixels_per_line + 7u) / 8u;
  unsigned y;
  for (y=0; y<height; ++y) {
    const uint8_t *input = rgb + (size_t)y * bytes_per_line;
    uint8_t *output = monochrome + (size_t)y * packed_bytes_per_line;
    unsigned x;
    unsigned output_byte = 0;
    uint8_t mask = 0x80u;
    uint8_t packed = 0;
    for (x=0; x<pixels_per_line; ++x) {
      const unsigned rgb_offset = x * 3u;
      const unsigned brightness = (unsigned)input[rgb_offset] +
          input[rgb_offset+1u] + input[rgb_offset+2u];
      const int32_t matrix_entry = table->matrix[
          (y % table->dimension) * table->dimension +
          (x % table->dimension)];
      const double threshold = (double)matrix_entry / table->divisor;
      if (!(threshold < (double)brightness / 765.0)) packed |= mask;
      if (mask == 1u || x+1u == pixels_per_line) {
        output[output_byte++] = packed;
        packed = 0;
        mask = 0x80u;
      } else {
        mask >>= 1;
      }
    }
  }
}
