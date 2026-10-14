/* Original document lifecycle -> already-reconstructed job callbacks.
 * Neither StartDocument nor EndDocument is reimplemented here.
 * The original raster2printer performs a by-value 0x10a8-byte copy
 * for each document function, so EndDocument zeroes ONLY that copy,
 * although it closes the actual cupsRaster stream.
 */
#include "original_raster_document_bridge.h"
#include <string.h>

void classic_original_bridge_start_document(void *ctx,
                                            const unsigned char *descriptor,
                                            unsigned descriptor_size) {
    classic_original_raster_document_bridge *bridge=ctx;
    (void)descriptor_size; /* Original caller always passes 0x10a8 bytes. */
    classic_original_start_document(&bridge->original,descriptor);
}
void classic_original_bridge_end_document(void *ctx,
                                          const void *descriptor,
                                          size_t descriptor_size) {
    classic_original_raster_document_bridge *bridge=ctx;
    unsigned char by_value[CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE];
    (void)descriptor_size;
    memcpy(by_value,descriptor,sizeof by_value);
    classic_original_end_document(&bridge->original,by_value);
    /* Original caller's descriptor is deliberately left unchanged. */
}
