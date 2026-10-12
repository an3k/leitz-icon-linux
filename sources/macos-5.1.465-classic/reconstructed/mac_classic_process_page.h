/* macOS classic 5.1.465 x86_64 ProcessPage reconstructed row/blank fragments.
 * Binary source evidence: raster2printer, _ProcessPage 0x100007560..0x1000089a6.
 * This is an isolated C module, not a complete recreation of ProcessPage.
 */
#ifndef MAC_CLASSIC_PROCESS_PAGE_H
#define MAC_CLASSIC_PROCESS_PAGE_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
  unsigned optimization;  /* Only original modes 2 and 3 currently recovered */
  unsigned stride;        /* Bytes per unpacked/dithered row */
  unsigned previous_width;
  unsigned previous_offset;
  unsigned pending_blank;
  unsigned first_nonblank;
} mac_classic_raster_row;

/* Initializes state for one page. It does not send page start commands.
 * Mode 2: run-length compression if smaller than raw data.
 * Mode 3: raw data with trimmed white bytes on either side.
 */
int mac_classic_row_init(mac_classic_raster_row *s, unsigned mode, unsigned stride);

/* Encode one row of bytes into out[capacity]. Return 0 on success, -1 on error.
 * White-only rows remain pending, and produce no output.
 * The caller passes one complete row (stride bytes) and reuses the state.
 */
int mac_classic_row_encode(mac_classic_raster_row *s, const uint8_t *row,
                           uint8_t *out, size_t capacity, size_t *used);

/* The original's optimized modes do not flush trailing white rows in the
 * ProcessPage exit block. Return the count solely for auditing, no output.
 */
unsigned mac_classic_pending_trailing_blank(const mac_classic_raster_row *s);
#endif
