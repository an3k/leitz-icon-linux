#ifndef CLASSIC_ORIGINAL_RASTER_CONNECTED_JOB_H
#define CLASSIC_ORIGINAL_RASTER_CONNECTED_JOB_H

#include "original_raster_option_initialization.h"
#include "original_raster_active_page_read.h"
#include "original_raster_active_page_output.h"
#include "original_raster_job_epilogue.h"

/* Connects EXISTING recovered manufacturer branches into one callable C job.
 * Callback descriptors emulate CUPS and OEM helpers for source verification.
 * This is NOT the original C++ ABI or a production printer filter.
 */
typedef struct {
    classic_oem_option_initialization_io initialization;
    classic_oem_page_output_io page_output;
    classic_original_raster_finish_ops finish;
    int (*is_terminated)(void *context);
    void *termination_context;
    int socket_fd;
    uint32_t *job_size;
} classic_original_raster_connected_io;

/* Exposed page state is intentional: original code can leave a live
 * pageBuffer after a zero-pixel read or interrupted dispatch. The caller
 * must account for its ownership during tests; the reconstruction does
 * NOT silently change manufacturer cleanup behavior.
 */
int classic_original_raster_connected_job(
    const classic_original_raster_connected_io *io,
    int argc,char **argv,
    classic_oem_read_descriptor *descriptor,
    classic_oem_active_page_state *page_state,
    unsigned *page_index);

#endif
