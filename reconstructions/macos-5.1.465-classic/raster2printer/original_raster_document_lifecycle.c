/* Leitz Icon macOS 5.1.465 OEM StartDocument and EndDocument.
 * Recover complete observable native sequence from SHA-256-verified
 * raster2printer Mach-O at 0x100006890..0x100006dbb and
 * 0x100008ac0..0x100008cdc. Reuse the previously reconstructed
 * manufacturer command-byte builders; do NOT duplicate their logic.
 *
 * The original ignores _WriteData's return code. It flushes only
 * after the optional ESC E in _EndDocument, not after ESC @ ESC Z.
 * It closes CUPS unconditionally and zeros the descriptor's first
 * 8-byte raster-stream pointer, even on termination.
 */
#include "original_raster_document_lifecycle.h"
#include "raster2printer_boundaries_recovered.h"
#include <string.h>

static void emit_oem(const classic_original_document_ops *io,
                     const uint8_t *bytes,size_t length,
                     const unsigned char *descriptor) {
    unsigned char copy[CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE];
    memcpy(copy,descriptor,sizeof copy);
    *io->job_size += (uint32_t)length;
    (void)io->write_data(io->context,bytes,length,copy,sizeof copy,
                         io->current_socket_fd ?
                         io->current_socket_fd(io->context) : io->socket_fd);
}
void classic_original_start_document(const classic_original_document_ops *io,
                                     const unsigned char *descriptor) {
    uint32_t resolution;
    memcpy(&resolution,descriptor+0x14,sizeof resolution);
    uint8_t bytes[124];
    size_t total=classic_raster_start_document(resolution!=0,bytes);
    /* OEM _WriteData calls are separate, never one 124-byte write. */
    emit_oem(io,bytes,122,descriptor);
    if(total==124)
        emit_oem(io,bytes+122,2,descriptor);
}
void classic_original_end_document(const classic_original_document_ops *io,
                                   unsigned char *descriptor) {
    uint32_t cutter=0;
    memcpy(&cutter,descriptor+0x10,sizeof cutter);
    int termination=io->terminated(io->context);
    uint8_t bytes[6];
    size_t total=classic_raster_end_document(cutter!=0,termination,bytes);
    if(total==6) {
        emit_oem(io,bytes,2,descriptor); /* ESC E */
        io->flush_data_streams(io->context);
        emit_oem(io,bytes+2,4,descriptor);
    } else {
        emit_oem(io,bytes,4,descriptor);
    }
    void *raster=NULL;
    memcpy(&raster,descriptor,sizeof raster);
    io->close_raster(io->context,raster);
    memset(descriptor,0,sizeof raster);
}
