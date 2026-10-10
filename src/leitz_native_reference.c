/* Independent wire-format reference for Esselte's 2014 Linux v1.0.3 driver.
 * Distinct from production leitz_codec.c. No USB access or CUPS dependencies.
 */
#include "leitz_native_reference.h"
#include <string.h>

#define ESC 0x1bu
#define SYN 0x16u

static int send(FILE *f, const uint8_t *data, size_t len) {
  return fwrite(data, 1, len, f) == len ? 0 : -1;
}
static int skip_rows(leitz_native_reference *p, unsigned rows) {
  while (rows) {
    unsigned step = rows > 65535u ? 65535u : rows;
    const uint8_t command[4] = {ESC, 'f', (uint8_t)(step >> 8), (uint8_t)step};
    if (send(p->out, command, sizeof command)) return -1;
    rows -= step;
  }
  return 0;
}
int leitz_native_begin(leitz_native_reference *p, FILE *out,
                       unsigned width, unsigned rows, unsigned dpi_y,
                       unsigned length_units) {
  if (!p || !out || !width || width > 960 || !rows || rows > 64000 ||
      (dpi_y != 300 && dpi_y != 600) || length_units > 65535)
    return -1;
  memset(p, 0, sizeof *p);
  p->out = out;
  p->width = width;
  p->total_rows = rows;
  p->dpi_y = dpi_y;
  const uint8_t init[4] = {ESC, '@', ESC, dpi_y == 600 ? 'i' : 'h'};
  uint8_t preamble[120];
  memset(preamble, ESC, sizeof preamble);
  const uint8_t setup[11] = {ESC, 'L', (uint8_t)(length_units >> 8),
    (uint8_t)length_units, ESC, 'w', (uint8_t)(width >> 8),
    (uint8_t)width, ESC, 'B', 0};
  if (send(out, init, sizeof init) ||
      send(out, preamble, sizeof preamble) ||
      send(out, setup, sizeof setup)) return p->failed = -1;
  return 0;
}
int leitz_native_row(leitz_native_reference *p, const uint8_t *packed,
                     unsigned row_bytes) {
  if (!p || p->failed || p->finished || !packed || p->rows_written >= p->total_rows ||
      row_bytes != (p->width + 7) / 8) return -1;
  p->rows_written++;
  unsigned ink = 0;
  for (unsigned i = 0; i < row_bytes; i++) ink |= packed[i];
  if (!ink) {
    p->pending_blank++;
    return 0;
  }
  if (skip_rows(p, p->pending_blank)) return p->failed = -1;
  p->pending_blank = 0;
  const uint8_t header[4] = {ESC, 'D', (uint8_t)row_bytes, SYN};
  if (send(p->out, header, sizeof header) ||
      send(p->out, packed, row_bytes)) return p->failed = -1;
  return 0;
}
int leitz_native_end(leitz_native_reference *p) {
  if (!p || p->failed || p->finished || p->rows_written != p->total_rows) return -1;
  /* Mirror the original Linux 1.0.3 raw/optimization-1 stream:
   * trailing blank rows remain pending at page end. The source emits
   * ESC f only when a subsequent nonblank row is encountered.
   */
  const uint8_t finish[4] = {ESC, 'G', ESC, 'E'};
  if (send(p->out, finish, sizeof finish)) return p->failed = -1;
  p->finished = 1;
  return 0;
}
