/* Derived independently from 5.1.465 classic Mach-O raster2printer.asm,
 * _StartDocument 0x100006890, _EndPage 0x1000089b0 and _EndDocument 0x100008ac0.
 * This does not recreate the whole raster2printer executable.
 */
#include "mac_classic_raster_boundaries.h"

size_t mac_classic_start_document(int high_resolution, uint8_t out[124]) {
  if (!out) return 0;
  /* 120 ESC padding bytes plus ESC @ = 0x7a bytes per binary. */
  for (size_t i = 0; i < 121; ++i) out[i] = 0x1b;
  out[121] = (uint8_t)'@';
  if (!high_resolution) return 122;
  out[122] = 0x1b; out[123] = (uint8_t)'i';
  return 124;
}

size_t mac_classic_start_page(uint16_t raster_width, uint16_t raster_height,
                              int high_resolution, uint8_t out[11]) {
  if (!out) return 0;
  unsigned length=((unsigned)raster_height+(high_resolution?60u:30u)) /
                  (high_resolution?2u:1u);
  out[0]=0x1b; out[1]=(uint8_t)'L';
  out[2]=(uint8_t)(length>>8); out[3]=(uint8_t)length;
  out[4]=0x1b; out[5]=(uint8_t)'w';
  out[6]=(uint8_t)(raster_width>>8); out[7]=(uint8_t)raster_width;
  out[8]=0x1b; out[9]=(uint8_t)'B'; out[10]=0;
  return 11;
}

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
