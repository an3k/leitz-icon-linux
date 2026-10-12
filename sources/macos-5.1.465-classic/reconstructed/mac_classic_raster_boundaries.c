/* Derived independently from 5.1.465 classic Mach-O raster2printer.asm,
 * _EndPage 0x1000089b0 and _EndDocument 0x100008ac0.
 * This does not recreate the whole raster2printer executable.
 */
#include "mac_classic_raster_boundaries.h"

size_t mac_classic_end_page(int cut_every_label, uint8_t out[4]) {
  if (!out) return 0;
  out[0] = 0x1b; out[1] = (uint8_t)'G';
  if (!cut_every_label) return 2;
  out[2] = 0x1b; out[3] = (uint8_t)'E';
  return 4;
}

size_t mac_classic_end_document(int cut_every_label, int terminate_flag, uint8_t out[6]) {
  if (!out) return 0;
  size_t n = 0;
  if (!cut_every_label && !terminate_flag) {
    out[n++] = 0x1b; out[n++] = (uint8_t)'E';
  }
  out[n++] = 0x1b; out[n++] = (uint8_t)'@';
  out[n++] = 0x1b; out[n++] = (uint8_t)'Z';
  return n;
}
