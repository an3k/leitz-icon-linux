#include "raster2printer_boundaries_recovered.h"

/* OEM _StartDocument, 0x100006890: 120 ESC padding bytes, ESC @,
 * and (at 600 dpi) ESC i. No additional commands or behavior added. */
size_t classic_raster_start_document(int high_resolution, uint8_t out[124]) {
  size_t i;
  for (i = 0; i < 121; i++) out[i] = 0x1b;
  out[i++] = '@';
  if (high_resolution) { out[i++] = 0x1b; out[i++] = 'i'; }
  return i;
}

/* OEM _StartPage, 0x100007380. Geometry sourced from the original raster
 * header; no fixed length is imposed. */
size_t classic_raster_start_page(uint16_t width, uint16_t height,
                                int high_resolution, uint8_t out[11]) {
  /* Native 0x1000073d6..0x100007405 stores the sum as uint16_t
   * BEFORE the optional divide-by-two. Ordinary media are unaffected;
   * 16-bit overflow must match the original manufacturer code. */
  uint16_t length = (uint16_t)((unsigned)height +
                                (high_resolution ? 60u : 30u));
  if (high_resolution) length = (uint16_t)(length / 2u);
  out[0]=0x1b;out[1]='L';out[2]=(uint8_t)(length >> 8);out[3]=(uint8_t)length;
  out[4]=0x1b;out[5]='w';out[6]=(uint8_t)(width >> 8);out[7]=(uint8_t)width;
  out[8]=0x1b;out[9]='B';out[10]=0;
  return 11;
}

/* OEM _EndPage, 0x1000089b0..0x100008ab1. The caller submits output
 * using _WriteData then always invokes _FlushDataStreams. */
size_t classic_raster_end_page(int cut_every_label, uint8_t out[4]) {
  out[0]=0x1b;out[1]='G';
  if (!cut_every_label) return 2;
  out[2]=0x1b;out[3]='E';
  return 4;
}

/* OEM _EndDocument, 0x100008ac0..0x100008cdc.
 * Closing the CUPS raster handle is a separate original side effect and is
 * not reproduced by this output-command-only function. */
size_t classic_raster_end_document(int cut_every_label, int terminate_flag,
                                   uint8_t out[6]) {
  size_t n = 0;
  if (!cut_every_label && !terminate_flag) {
    out[n++]=0x1b;out[n++]='E';
  }
  out[n++]=0x1b;out[n++]='@';out[n++]=0x1b;out[n++]='Z';
  return n;
}

/* OEM _FlushDataStreams 0x1000015f0..0x10000162c.
 * The OEM does NOT check either fflush() return value. */
void classic_raster_flush_data_streams(FILE *stream, FILE *debug_dump) {
  fflush(stream);
  if (debug_dump) fflush(debug_dump);
}
