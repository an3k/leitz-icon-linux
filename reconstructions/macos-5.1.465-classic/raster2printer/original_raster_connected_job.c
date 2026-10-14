/* Manufacturer-derived macOS classic raster2printer 5.1.465.
 * CONNECTOR of previously recovered distinct original _raster2printer
 * fragments: preflight/options/_StartDocument, active CUPS page read,
 * normal page output, final _EndDocument and job epilogue.
 *
 * The connections and callback API are newly authored C scaffolding, not
 * manufacturer source or proof of original whole-function control flow.
 * Reprint/IPP status, native ABI and undefined allocation branches are
 * deliberately outside the verified scope. Existing implementations are
 * reused, not cloned or overwritten.
 */
#include "original_raster_connected_job.h"
#include <string.h>

int classic_original_raster_connected_job(
    const classic_original_raster_connected_io *io,
    int argc,char **argv,
    classic_oem_read_descriptor *descriptor,
    classic_oem_active_page_state *page_state,
    unsigned *page_index) {
    classic_oem_preflight_state input={0};
    int early_result=0;

    /* The manufacturer's descriptor is 0x10a8 bytes and must be cleared
     * before parsing the PPD. Do not initialize it twice. */
    if(!classic_original_raster_initialize_job(&io->initialization,
        argc,argv,(unsigned char *)descriptor,&input,&early_result))
        return early_result;

    descriptor->cups_raster=(cups_raster_t *)input.raster;
    memset(page_state,0,sizeof *page_state);
    *page_index=0;
    int process_result=0;
    cups_page_header2_t header;
    for (;;) {
        if (io->is_terminated &&
            io->is_terminated(io->termination_context))
            break;

        /* Separate original inline reader, NOT exported _ReadPage. */
        int has_page=classic_original_raster_active_page_read(
            descriptor,&header,page_state);
        if (!has_page)
            break;

        int terminated=io->is_terminated ?
            io->is_terminated(io->termination_context):0;
        if (!classic_original_raster_output_page(
            &io->page_output,descriptor,page_state,page_index,
            has_page,terminated,&process_result))
            break;
    }

    classic_original_raster_finish_state finish={
        (const unsigned char *)descriptor,
        input.input_fd,
        io->socket_fd,
        io->is_terminated ?
            io->is_terminated(io->termination_context):0,
        process_result,
        io->job_size
    };
    return classic_original_raster_finish_job(&io->finish,&finish);
}
