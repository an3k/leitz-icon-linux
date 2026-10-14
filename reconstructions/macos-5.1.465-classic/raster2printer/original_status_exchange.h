#ifndef CLASSIC_ORIGINAL_STATUS_EXCHANGE_H
#define CLASSIC_ORIGINAL_STATUS_EXCHANGE_H
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
#include "original_raster_document_lifecycle.h"

/* _CheckPrinterStatus, OEM classic raster2printer 0x100009190..0x100009a2e.
 * Callback interface is a NEW reconstruction aid, not the original ABI.
 * No live printer, socket, or CUPS filter uses this module.
 */
typedef struct {
    void *context;
    const char *device_scheme;     /* original descriptor +0x50 */
    int socket_fd;
    int resolved_uri_present;
    int (*termination_flag)(void *);
    int (*resolve_device_uri)(void *);
    /* OEM send(fd, ESC U, 2, 0) / recv(fd, reply, 64, 0). */
    ssize_t (*send_once)(void *,int,const void *,size_t,int);
    ssize_t (*recv_once)(void *,int,void *,size_t,int);
    int (*put_character)(void *,int);
    int (*flush_stdout)(void *);
    ssize_t (*read_backchannel)(void *,void *,size_t,double);
    void (*report)(void *,const char *);
    /* Original _WriteData receives the full descriptor BY VALUE and the
     * current native socket descriptor; its result is ignored. */
    int (*write_data)(void *,const uint8_t *,size_t,const void *,size_t,int);
    void (*sleep_seconds)(void *,unsigned);
    unsigned *job_size;
    int (*last_error)(void *);
    const unsigned char *descriptor; /* original 0x10a8-byte descriptor */
} classic_original_status_io;

/* Return 0/1 as the OEM for the defined read paths. Analysis-only -2 marks
 * original undefined/unverified branches: e.g. short socket send, 64-byte
 * unterminated response. -2 is NOT a manufacturer return value or a new
 * error-handling policy; those paths are outside verified reconstruction.
 */
int classic_original_check_status(const classic_original_status_io *io);
#endif
