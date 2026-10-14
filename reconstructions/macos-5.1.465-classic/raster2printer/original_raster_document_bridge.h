#ifndef CLASSIC_ORIGINAL_RASTER_DOCUMENT_BRIDGE_H
#define CLASSIC_ORIGINAL_RASTER_DOCUMENT_BRIDGE_H

#include "original_raster_document_lifecycle.h"

/* A callback bridge between the already existing reconstructed
 * raster job and the independently recovered original StartDocument
 * and EndDocument functions. Not an OEM C++ ABI.
 *
 * Place this as the first member of a user-defined callback context
 * passed to original_raster_option_initialization and finish_job.
 * The embedded document callbacks may use a distinct .context.
 */
typedef struct {
    classic_original_document_ops original;
} classic_original_raster_document_bridge;

/* Matches existing original_raster_option_initialization.start_document. */
void classic_original_bridge_start_document(void *ctx,
                                            const unsigned char *descriptor,
                                            unsigned descriptor_size);

/* Matches existing original_raster_job_epilogue.end_document. OEM passes
 * the descriptor BY VALUE, so CUPS pointer reset is local to the copy. */
void classic_original_bridge_end_document(void *ctx,
                                          const void *descriptor,
                                          size_t descriptor_size);
#endif
