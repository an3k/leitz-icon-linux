#ifndef CLASSIC_ORIGINAL_STATUS_EXCHANGE_H
#define CLASSIC_ORIGINAL_STATUS_EXCHANGE_H
#include <stddef.h>
#include <sys/types.h>

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
    ssize_t (*send_once)(void *,int,const void *,size_t);
    ssize_t (*recv_once)(void *,int,void *,size_t);
    int (*put_character)(void *,int);
    int (*flush_stdout)(void *);
    ssize_t (*read_backchannel)(void *,void *,size_t,double);
    void (*report)(void *,const char *);
    void (*write_data)(void *,const void *,size_t);
    void (*sleep_seconds)(void *,unsigned);
    unsigned *job_size;
    int (*last_error)(void *);
} classic_original_status_io;

/* Return 0/1 as the OEM for the defined read paths. Analysis-only -2 marks
 * original undefined/unverified branches: e.g. short socket send, 64-byte
 * unterminated response. -2 is NOT a manufacturer return value or a new
 * error-handling policy; those paths are outside verified reconstruction.
 */
int classic_original_check_status(const classic_original_status_io *io);
#endif
