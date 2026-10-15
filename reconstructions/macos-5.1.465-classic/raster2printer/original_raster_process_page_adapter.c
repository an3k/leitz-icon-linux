/* Reuse the individually recovered ProcessPage control loops without
 * rewriting either manufacturer's row encoding algorithm. */
#include "original_raster_process_page_adapter.h"
#include <string.h>

void classic_original_process_page_adapter_initialize(
    classic_original_process_page_adapter *adapter,
    classic_original_page_lifecycle_ops output,
    int (*is_terminated)(void *),void *termination_context) {
    memset(adapter,0,sizeof *adapter);
    adapter->modes01.output=output;
    adapter->modes01.is_terminated=is_terminated;
    adapter->modes01.termination_context=termination_context;
    adapter->modes23.output=output;
    adapter->modes23.job_cache=&adapter->cache;
    adapter->modes23.is_terminated=is_terminated;
    adapter->modes23.termination_context=termination_context;
}
int classic_original_process_page_callback(
    void *ctx,classic_oem_read_descriptor *descriptor,
    const uint8_t *pixels,unsigned page_index,int reprint) {
    classic_original_process_page_adapter *adapter=ctx;
    uint32_t mode;
    memcpy(&mode,(const uint8_t *)descriptor+0x40,sizeof mode);
    if (mode<=1u) {
        /* The fourth OEM ProcessPage parameter influences the width
         * resend condition in mode 2 only (0x10000815f..0x10000824d).
         * The already reconstructed mode-0/1 row paths do not consult
         * that argument; refusing these calls was an adapter invention. */
        return classic_original_process_page_modes01(
            &adapter->modes01,descriptor,pixels,page_index);
    }
    if (mode==2u || mode==3u) {
        /* For mode 2 the OEM's sole width source is the original
         * process-static 32-bit symbol (read at native 0x815f).
         * The independently recovered row algorithm continues to
         * consume its existing cache value; synchronize that field
         * from the authentic named global, never initialize the
         * OEM static cache to zero at every new page/job.
         * The OEM width store at 0x821f is reproduced at the exact
         * ESC D output site in the original modes23 source module.
         * Mode 3 still owns its separate descriptor-width cache. */
        if (mode==2u)
            adapter->cache.mode2_previous_width=
                classic_original_mode2_static_width;
        /* Page-specific OEM flag, not persistent job state. Keep the
         * shared width cache; do not leak reprint into later pages. */
        classic_original_process_modes23_io page_io=adapter->modes23;
        page_io.reprint=(unsigned)(reprint!=0);
        return classic_original_process_page_modes23(
            &page_io,descriptor,pixels,page_index);
    }
    return -2;
}
