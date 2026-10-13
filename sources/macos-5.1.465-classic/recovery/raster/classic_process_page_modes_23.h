#ifndef CLASSIC_PROCESS_PAGE_MODES_23_H
#define CLASSIC_PROCESS_PAGE_MODES_23_H
#include <stddef.h>
#include <stdint.h>

/*
 * Isolated manufacturer-derived row paths from classic _ProcessPage,
 * Mach-O 0x100007560..0x1000089a6. This is not the original C ABI.
 *
 * mode 2: trimmed rows, optional 0x17 bit-run encoding;
 * mode 3: trimmed/raw rows, with asymmetric trailing-trim condition.
 *
 * The original keeps mode-2's static cached width and mode-3's descriptor
 * width across pages, while the previous left offset is local to a page.
 */
typedef struct {
    unsigned mode2_previous_width;
    unsigned mode3_previous_width;
} classic_mode23_job_cache;

typedef struct {
    classic_mode23_job_cache *job;
    unsigned mode;
    unsigned packed_stride;
    unsigned previous_left_offset;
    uint16_t pending_empty_rows;
    unsigned first_nonblank;
    unsigned reprint;  /* original fourth ProcessPage argument */
} classic_mode23_page;

/* Reuses the job's width caches; starts a fresh page-local offset/blank run. */
void classic_mode23_begin(classic_mode23_page *page,
                          classic_mode23_job_cache *job,
                          unsigned mode, unsigned packed_stride,
                          unsigned reprint);

/* Encodes a single raster row. The caller owns the output and must provide
 * at least packed_stride + 11 bytes. This does not impose a fixed label
 * length, add output-buffer validation, or flush trailing blank rows.
 *
 * The exact manufacturer out-of-range behavior, full original descriptor
 * memory layout and certain termination/error branches remain unverified.
 */
size_t classic_mode23_row(classic_mode23_page *page,
                          const uint8_t *row, uint8_t *out);

#endif
