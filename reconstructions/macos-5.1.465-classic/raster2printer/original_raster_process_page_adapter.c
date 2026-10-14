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
    if(reprint!=0) return -2;
    if (mode<=1u)
        return classic_original_process_page_modes01(
            &adapter->modes01,descriptor,pixels,page_index);
    if (mode==2u || mode==3u)
        return classic_original_process_page_modes23(
            &adapter->modes23,descriptor,pixels,page_index);
    return -2;
}
