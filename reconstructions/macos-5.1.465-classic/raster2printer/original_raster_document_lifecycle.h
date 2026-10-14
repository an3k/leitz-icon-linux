#ifndef ORIGINAL_RASTER_DOCUMENT_LIFECYCLE_H
#define ORIGINAL_RASTER_DOCUMENT_LIFECYCLE_H
#include <stddef.h>
#include <stdint.h>

/* OEM macOS classic raster2printer 5.1.465, _StartDocument
 * 0x100006890..0x100006dbb and _EndDocument 0x100008ac0..0x100008cdc.
 * The original passes the 0x10a8-byte descriptor by VALUE.
 * This injectable C callback ABI is reconstruction scaffolding, not
 * the original binary's x86-64 calling convention. */
enum { CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE=0x10a8 };
typedef struct {
    void *context;
    int (*write_data)(void *,const uint8_t *,size_t,const void *,size_t,int);
    void (*flush_data_streams)(void *);
    void (*close_raster)(void *,void *);
    uint32_t *job_size;
    int socket_fd;
    int (*terminated)(void *);
} classic_original_document_ops;

/* These functions represent the exact defined manufacturer call sequence.
 * _StartDocument never calls flush or closes CUPS.
 * _EndDocument always invokes cupsRasterClose and clears descriptor+0,
 * including on a terminated job, without validating the CUPS pointer. */
void classic_original_start_document(const classic_original_document_ops *io,
                                     const unsigned char *descriptor);
void classic_original_end_document(const classic_original_document_ops *io,
                                   unsigned char *descriptor);
#endif
