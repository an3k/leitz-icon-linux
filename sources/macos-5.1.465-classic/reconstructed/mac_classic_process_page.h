/* Classic macOS raster2printer 5.1.465: isolated ProcessPage row paths.
 * Source: Mach-O x86-64 0x100007560..0x1000089a6.
 * NOT a complete recovered filter or a device-ready implementation.
 */
#ifndef MAC_CLASSIC_PROCESS_PAGE_H
#define MAC_CLASSIC_PROCESS_PAGE_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
  unsigned optimization;  /* Original modes 0, 1, 2, 3 */
  unsigned stride;        /* Bytes in a packed monochrome row, 1..120 */
  unsigned previous_width;
  unsigned previous_offset;
  unsigned pending_blank;
  unsigned first_nonblank;
} mac_classic_raster_row;

int mac_classic_row_init(mac_classic_raster_row *s, unsigned mode, unsigned stride);

/* In original modes 0 and 1, ProcessPage emits a single ESC D width SYN
 * before the first raster row. Modes 2 and 3 have no initial row command.
 * Return number of bytes written (0 or 4); 0 with a null output indicates
 * invalid input, and is not a success indication for modes 0/1.
 */
size_t mac_classic_row_preamble(const mac_classic_raster_row *s,
                                uint8_t out[4]);

/* Encode one full packed bitmap row, using the same state for the page.
 * Empty rows are emitted verbatim by mode 0 and queued as feed by 1..3.
 * On failure no state is changed and *used remains zero.
 */
int mac_classic_row_encode(mac_classic_raster_row *s, const uint8_t *row,
                           uint8_t *out, size_t capacity, size_t *used);

/* In the original optimized modes, final white rows are not flushed by
 * ProcessPage, which is relevant to the page's mechanical length.
 */
unsigned mac_classic_pending_trailing_blank(const mac_classic_raster_row *s);
#endif
