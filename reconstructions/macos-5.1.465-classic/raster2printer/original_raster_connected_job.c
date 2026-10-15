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
#include <stdio.h>
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

    /* Original _raster2printer after _StartDocument:
     * 0x100005523 and 0x1000055c3, OEM source lines 256/258.
     * These are unconditional upon successful startup, including
     * a job terminated before its first page header. */
    fprintf(stderr,
        "\nraster2printer.c:256:raster2printer(): started printing pages");
    fprintf(stderr,
        "\nraster2printer.c:258:raster2printer(): started printing pages");

    /* CUPS raster was already copied into the OEM descriptor before
     * _StartDocument by original_raster_option_initialization.c. */
    memset(page_state,0,sizeof *page_state);
    *page_index=0;
    int process_result=0;
    cups_page_header2_t header;
    for (;;) {
        if (io->is_terminated &&
            io->is_terminated(io->termination_context))
            break;

        /* Original 0x100005603 initializes the reprint flag to zero.
         * Native OEM audits through 0x1000062e5 found no writes to
         * that flag in the page loop, so its reprint-true body at
         * 0x100006135..0x1000061d6 is unreachable here.
         * Its exact ProcessPage(...,1)/EndPage sequence is recovered
         * separately in classic_original_raster_reprint_page(), but
         * must not invent a new reprint trigger in the normal job.
         * This diagnostic also executes on the final EOF attempt. */
        fprintf(stderr,"\n Not reprinted i_page = %i.\n",
                (int)*page_index);

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
