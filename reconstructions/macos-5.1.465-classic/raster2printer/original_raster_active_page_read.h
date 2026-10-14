#ifndef ORIGINAL_RASTER_ACTIVE_PAGE_READ_H
#define ORIGINAL_RASTER_ACTIVE_PAGE_READ_H
#include "original_read_page.h"
#include <stdint.h>

/* Manufacturer raster2printer _raster2printer, not the separate _ReadPage:
 * direct inline read and allocation region 0x1000056c3..0x100005f22.
 * Caller owns buffers and must arrange the original later cleanup.
 * One invocation starts with an unallocated new-page state. */
typedef struct {
    unsigned page_number;
    int32_t requested_bytes;
    uint32_t received_bytes;
    unsigned char *page_buffer;
    unsigned char *dithered_page;
} classic_oem_active_page_state;

/* Returns 1 when a page was read with positive pixel count; zero when
 * the original turns off its "has page" flag. Header EOF and pixel-read
 * zero are distinguishable by state->page_number / requested_bytes.
 * This does not reconstruct the reprint/previous-label branch or the
 * surrounding cancellation and device-status loop. */
int classic_original_raster_active_page_read(
    classic_oem_read_descriptor *descriptor,
    cups_page_header2_t *header,
    classic_oem_active_page_state *state);
#endif
