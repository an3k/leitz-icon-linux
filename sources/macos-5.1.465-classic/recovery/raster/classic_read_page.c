#include "classic_read_page.h"
#include <stdlib.h>

/* The OEM checks only cupsRasterReadHeader2's zero result at
 * 0x100008d46, then increments its counter at 0x100008d5f.
 * OEM allocation at 0x10000905e is calloc(1,height*bytes_per_line).
 * The 32-bit multiplication is reconstructed deliberately without an
 * added overflow guard: see 0x10000903a..0x100009051.
 * Exactly one cupsRasterReadPixels is observed, 0x10000906a..0x100009083.
 * A zero result (not a nonzero partial result) selects the failure path
 * at 0x100009086..0x1000090c0.
 *
 * No synthetic fallback for malformed pages, retry of partial reads,
 * or stricter short-read validation is introduced.
 */
int classic_read_page_core(const classic_page_source *source,
                           unsigned *page_counter,
                           classic_page_result *result) {
  if (!source->read_header(source->context, &result->header)) return 0;
  result->page_number = ++*page_counter;
  result->requested_bytes = result->header.cups_height *
                            result->header.cups_bytes_per_line;
  result->pixels = calloc(1, result->requested_bytes);
  if (result->pixels == NULL) return -1;
  result->received_bytes = source->read_pixels(
      source->context, result->pixels, result->requested_bytes);
  return result->received_bytes != 0 ? 1 : -1;
}
