#ifndef ORIGINAL_RASTER_PROCESS_PAGE_ADAPTER_H
#define ORIGINAL_RASTER_PROCESS_PAGE_ADAPTER_H

#include "original_raster_process_page_modes01.h"
#include "original_raster_process_page_modes23.h"

/* Source-level dispatch bridge for the existing
 * classic_oem_page_output_io.process_page callback.
 * Maintains the already reconstructed mode 2/3 width caches across pages.
 * NOT the original Mach-O C++ ABI. */
typedef struct {
    classic_original_process_modes01_io modes01;
    classic_original_process_modes23_io modes23;
    classic_mode23_job_cache cache;
} classic_original_process_page_adapter;

/* Initialize once for each OEM job. Shares output writer, job_size and
 * termination callbacks with the already recovered page/document path. */
void classic_original_process_page_adapter_initialize(
    classic_original_process_page_adapter *adapter,
    classic_original_page_lifecycle_ops output,
    int (*is_terminated)(void *),void *termination_context);

/* Matches classic_oem_page_output_io.process_page. For modes 0/1 the
 * original row implementation reads four bytes beyond its final row:
 * only call this with readable padding, otherwise OEM behavior is
 * undefined. Normal reprint flag is always 0 in the original job loop.
 * Nonzero reprint is refused with reconstruction-only -2. */
int classic_original_process_page_callback(
    void *adapter,classic_oem_read_descriptor *descriptor,
    const uint8_t *pixels,unsigned page_index,int reprint);
#endif
