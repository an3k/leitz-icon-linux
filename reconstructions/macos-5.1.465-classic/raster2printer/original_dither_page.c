/* Manufacturer macOS Classic raster2printer _DitherPage, 0x100006dd0..
 * 0x100007335, reconstructed from the original verified Intel Mach-O.
 * Restores the manufacturer's two unconditional diagnostic writes and the
 * actual descriptor argument layout. The RGB24 pixel loop and matrix choice
 * were reconstructed earlier in raster2printer_dither_recovered.c; reuse
 * that exact code instead of introducing a second implementation.
 * Original invalid quality and malformed pixels can produce undefined
 * memory access: the recovered function introduces no fallback behavior.
 */
#include "original_dither_page.h"
#include "raster2printer_dither_recovered.h"
#include <stdio.h>

void classic_original_dither_page(classic_oem_read_descriptor *descriptor,
                                  const uint8_t *rgb,
                                  uint8_t *monochrome,
                                  unsigned quality) {
    fprintf(stderr,"\nraster2printer.c:593:DitherPage(): Height = %i",
            (int)descriptor->cups_height);
    fprintf(stderr,"\nraster2printer.c:594:DitherPage(): Bytes per line = %i",
            (int)descriptor->cups_bytes_per_line);
    classic_raster_dither_rgb24_recovered(quality,rgb,descriptor->cups_height,
                                           descriptor->cups_bytes_per_line,
                                           monochrome);
}
