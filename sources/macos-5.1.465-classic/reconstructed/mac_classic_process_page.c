/* Independently recovered from classic macOS raster2printer Mach-O x86_64.
 * _ProcessPage at 0x100007560..0x1000089a6 (optimization modes 2 and 3).
 * Original evidence: output boundaries and compression selection at
 * 0x100007972-0x100007e69 (mode 3), 0x100007f32-0x10000887d
 * (mode 2), and blank skip at 0x100007939-0x100007a74.
 * Modes 0/1: 0x100007633-0x100007739 (initial ESC D width SYN),
 * mode 0: 0x10000889b-0x100008945 (full raw row),
 * mode 1: 0x100007e6e-0x100007f32 (raw row after blank-feed handling).
 *
 * The manufacturer code contains raw buffer pointer arithmetic. This C
 * reconstruction uses bounded memcpy and explicit command bytes instead.
 * That is a safety improvement, not a claim of instruction-level identity.
 */
#include "mac_classic_process_page.h"
#include <string.h>

#define ESC 0x1bu
#define SYN 0x16u
#define SYN_COMPRESSED 0x17u

static int append(uint8_t *out, size_t cap, size_t *at,
                  const uint8_t *data, size_t size) {
  if (*at > cap || size > cap - *at) return -1;
  memcpy(out + *at, data, size);
  *at += size;
  return 0;
}
static int push(uint8_t *out, size_t cap, size_t *at, uint8_t byte) {
  if (*at == cap) return -1;
  out[(*at)++] = byte;
  return 0;
}
int mac_classic_row_init(mac_classic_raster_row *s, unsigned mode,
                         unsigned stride) {
  if (!s || mode > 3 || stride == 0 || stride > 120) return -1;
  memset(s, 0, sizeof(*s));
  s->optimization = mode;
  s->stride = stride;
  s->first_nonblank = 1;
  return 0;
}

size_t mac_classic_row_preamble(const mac_classic_raster_row *s,
                                uint8_t out[4]) {
  if (!s || s->stride == 0 || s->stride > 120 || s->optimization > 3)
    return 0;
  if (s->optimization >= 2) return 0;
  if (!out) return 0;
  const uint8_t header[] = {ESC, 'D', (uint8_t)s->stride, SYN};
  memcpy(out, header, sizeof header);
  return sizeof header;
}

/* The original bit traversal goes from bit 7 down to bit 0 (_is_bit_set at
 * 0x100001630), groups same-color bits, and emits 0x00..0x7f for white
 * or 0x80..0xff for black, storing run length minus one. */
static size_t rle_bits(const uint8_t *row, unsigned bytes,
                       uint8_t *out, size_t capacity) {
  unsigned count = 0, color = 0;
  size_t n = 0;
  for (unsigned x = 0; x < bytes; ++x) {
    for (int b = 7; b >= 0; --b) {
      unsigned bit = (row[x] >> b) & 1u;
      if (count && (color != bit || count == 128)) {
        if (n == capacity) return capacity + 1;
        out[n++] = (uint8_t)((color ? 0x80u : 0u) | (count - 1u));
        count = 0;
      }
      color = bit;
      ++count;
    }
  }
  if (count) {
    if (n == capacity) return capacity + 1;
    out[n++] = (uint8_t)((color ? 0x80u : 0u) | (count - 1u));
  }
  return n;
}

int mac_classic_row_encode(mac_classic_raster_row *s, const uint8_t *row,
                           uint8_t *out, size_t capacity, size_t *used) {
  if (!s || !row || !out || !used || s->optimization > 3 ||
      s->stride == 0 || s->stride > 120) return -1;
  *used = 0;
  /* Mode 0 bypasses the ink test entirely (0x1000078dc -> 0x10000889b).
   * ESC D width SYN is sent once by mac_classic_row_preamble(). */
  if (s->optimization == 0) {
    if (capacity < s->stride) return -1;
    memcpy(out, row, s->stride);
    *used = s->stride;
    return 0;
  }
  unsigned left = 0;
  while (left < s->stride && row[left] == 0) ++left;
  if (left == s->stride) {
    if (s->pending_blank == 65535u) return -1;
    ++s->pending_blank;
    return 0;
  }
  /* Mode 1: raw full-width bytes, without a per-row ESC D/SYN header.
   * Its original header is emitted only at the beginning of ProcessPage.
   * All optimized modes share the prior ESC f blank-run handling. */
  if (s->optimization == 1) {
    size_t at = 0;
    if (s->pending_blank) {
      const uint8_t feed[] = {ESC, 'f', (uint8_t)(s->pending_blank >> 8),
                              (uint8_t)s->pending_blank};
      if (append(out, capacity, &at, feed, sizeof feed)) return -1;
    }
    if (append(out, capacity, &at, row, s->stride)) return -1;
    s->pending_blank = 0;
    *used = at;
    return 0;
  }
  unsigned right = s->stride;
  /* 0x100007ab4 mode 3 tests the leading offset before trimming end.
   * Mode 2 always trims trailing white bytes (0x1000080b6). */
  if (s->optimization == 2 || left != 0)
    while (right > left && row[right-1] == 0) --right;
  unsigned n = right - left;
  size_t at = 0;
  if (s->pending_blank) {
    const uint8_t feed[] = {ESC, 'f', (uint8_t)(s->pending_blank >> 8),
                            (uint8_t)s->pending_blank};
    if (append(out, capacity, &at, feed, sizeof feed)) return -1;
  }
  if (s->optimization == 2 || left != s->previous_offset) {
    const uint8_t offset[] = {ESC, 'B', (uint8_t)left};
    if (append(out, capacity, &at, offset, sizeof offset)) return -1;
  }
  /* In normal _ProcessPage (fourth argument 0), mode 2 emits ESC D
   * only when n differs from the process-static previous width:
   * 0x10000815f..0x10000818b. Its first-row exception requires the
   * inactive reprint argument. Mode 3 compares descriptor +0x24:
   * 0x100007c71..0x100007c87. Both widths survive page boundaries. */
  if (n != s->previous_width) {
    const uint8_t width[] = {ESC, 'D', (uint8_t)n};
    if (append(out, capacity, &at, width, sizeof width)) return -1;
  }
  if (s->optimization == 2) {
    uint8_t encoded[960];
    size_t count = rle_bits(row + left, n, encoded, sizeof encoded);
    if (count < n) {
      if (push(out, capacity, &at, SYN_COMPRESSED) ||
          append(out, capacity, &at, encoded, count)) return -1;
    } else {
      if (push(out, capacity, &at, SYN) ||
          append(out, capacity, &at, row + left, n)) return -1;
    }
  } else {
    if (push(out, capacity, &at, SYN) ||
        append(out, capacity, &at, row + left, n)) return -1;
  }
  s->pending_blank = 0;
  s->previous_width = n;
  s->previous_offset = left;
  s->first_nonblank = 0;
  *used = at;
  return 0;
}
unsigned mac_classic_pending_trailing_blank(const mac_classic_raster_row *s) {
  return s ? s->pending_blank : 0u;
}
