#ifndef ORIGINAL_RASTER_ACTIVE_PAGE_OUTPUT_H
#define ORIGINAL_RASTER_ACTIVE_PAGE_OUTPUT_H
#include "original_raster_active_page_read.h"
#include <stddef.h>
#include <stdint.h>

/* Original raster2printer normal, non-reprint page branch:
 * 0x100005f3b..0x1000062e5. This is an isolated analysis interface,
 * NOT the original C++ ABI and NOT the complete _ProcessPage function.
 */
typedef struct {
    void *context;
    void (*dither_page)(void *,classic_oem_read_descriptor *,
                        const uint8_t *,uint8_t *,unsigned);
    void (*start_page)(void *,classic_oem_read_descriptor *);
    int (*process_page)(void *,classic_oem_read_descriptor *,
                        const uint8_t *,unsigned,int);
    void (*end_page)(void *,const void *,size_t);
} classic_oem_page_output_io;

/* The original tests termination/has-page before this output branch.
 * Returns 1 only if OEM continues into its next page read.
 * Caller passes its own zero-based page index and retains the page header.
 * The original reprint flag is initialized to zero and has no writes in
 * the entire audited OEM page loop. */
int classic_original_raster_output_page(
    const classic_oem_page_output_io *io,
    classic_oem_read_descriptor *descriptor,
    classic_oem_active_page_state *page,
    unsigned *page_index,
    int has_page,int terminated,
    int *last_process_result);
/* Original _raster2printer 0x100006135..0x1000061d6:
 * reprint-only ProcessPage(...,1) / EndPage(by value) sequence.
 * This branch is dormant in the actual OEM's reachable page loop.
 * Caller retains ownership of the buffers; no new reprint trigger,
 * validation or memory-free semantics are invented. */
int classic_original_raster_reprint_page(
    const classic_oem_page_output_io *io,
    classic_oem_read_descriptor *descriptor,
    const classic_oem_active_page_state *page,
    unsigned page_index);
#endif
