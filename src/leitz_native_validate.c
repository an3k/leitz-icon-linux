/* Strict single-label validator for the native Linux v1.0.3 plain/raw subset.
 * It is intentionally NOT installed as a CUPS backend and never touches USB.
 */
#include "leitz_native_reference.h"
#include <stdio.h>
#include <string.h>

static int bad(char *msg, size_t cap, const char *reason) {
  if (msg && cap) snprintf(msg, cap, "%s", reason);
  return -1;
}
static int match(FILE *f, const uint8_t *bytes, unsigned n) {
  for (unsigned i = 0; i < n; i++) if (fgetc(f) != bytes[i]) return -1;
  return 0;
}
int leitz_native_validate(FILE *in, unsigned *width_out, unsigned *rows_out,
                          char *error, size_t error_size) {
  if (!in || !width_out || !rows_out) return bad(error, error_size, "Missing inputs");
  *width_out = *rows_out = 0;
  const uint8_t reset[] = {0x1b, '@', 0x1b};
  if (match(in, reset, sizeof reset)) return bad(error, error_size, "No native reset");
  int dpi_mode = fgetc(in);
  if (dpi_mode != 'h' && dpi_mode != 'i') return bad(error, error_size, "Bad native resolution");
  for (unsigned i = 0; i < 120; i++)
    if (fgetc(in) != 0x1b) return bad(error, error_size, "Bad native page preamble");
  const uint8_t len_cmd[] = {0x1b, 'L'};
  if (match(in, len_cmd, sizeof len_cmd)) return bad(error, error_size, "Missing label length");
  int length_hi = fgetc(in), length_lo = fgetc(in);
  if (length_hi < 0 || length_lo < 0) return bad(error, error_size, "Truncated label length");
  const uint8_t width_cmd[] = {0x1b, 'w'};
  if (match(in, width_cmd, sizeof width_cmd)) return bad(error, error_size, "Missing print width");
  int hi = fgetc(in), lo = fgetc(in);
  if (hi < 0 || lo < 0) return bad(error, error_size, "Truncated print width");
  unsigned width = (unsigned)hi * 256 + (unsigned)lo;
  if (!width || width > 960) return bad(error, error_size, "Bad print width");
  const uint8_t b0[] = {0x1b, 'B', 0};
  if (match(in, b0, sizeof b0)) return bad(error, error_size, "Missing native B0");
  unsigned rows = 0, printed = 0;
  for (;;) {
    int a = fgetc(in), b = fgetc(in);
    if (a != 0x1b || b < 0) return bad(error, error_size, "Unexpected or missing command");
    if (b == 'G') {
      const uint8_t end[] = {0x1b, 'E'};
      if (match(in, end, sizeof end) || fgetc(in) != EOF)
        return bad(error, error_size, "Invalid native footer");
      if (!printed) return bad(error, error_size, "Empty printed label");
      *width_out = width;
      *rows_out = rows;
      if (error && error_size) snprintf(error, error_size, "OK");
      return 0;
    }
    if (b == 'f') {
      int fh = fgetc(in), fl = fgetc(in);
      if (fh < 0 || fl < 0) return bad(error, error_size, "Truncated feed command");
      unsigned count = (unsigned)fh * 256 + (unsigned)fl;
      if (!count || count > 64000 - rows) return bad(error, error_size, "Bad feed length");
      rows += count;
      continue;
    }
    if (b != 'D') return bad(error, error_size, "Unsupported native row command");
    int n = fgetc(in);
    if (n != (int)((width + 7) / 8) || fgetc(in) != 0x16)
      return bad(error, error_size, "Bad native raster row header");
    if (rows >= 64000) return bad(error, error_size, "Too many raster rows");
    int ink = 0;
    for (int i = 0; i < n; i++) {
      int v = fgetc(in);
      if (v < 0) return bad(error, error_size, "Truncated raster row");
      ink |= v;
    }
    if (!ink) return bad(error, error_size, "Unoptimized blank row");
    rows++;
    printed++;
  }
}
