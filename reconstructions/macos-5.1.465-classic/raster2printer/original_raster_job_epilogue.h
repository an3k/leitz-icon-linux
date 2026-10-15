#ifndef ORIGINAL_RASTER_JOB_EPILOGUE_H
#define ORIGINAL_RASTER_JOB_EPILOGUE_H
#include <stddef.h>
#include <stdint.h>

/* Recovered normal/job-exit branch of the original Leitz 5.1.465
 * raster2printer _raster2printer, 0x1000062eb..0x10000685a.
 * This callback interface is reconstruction scaffolding, not the
 * manufacturer's x86_64 ABI or a new printer backend. */
enum { CLASSIC_OEM_DESCRIPTOR_BYTES = 0x10a8 };
typedef struct {
    void *context;
    void (*end_document)(void *, const void *, size_t);
    int (*close_fd)(void *, int);
    void (*close_debug_file)(void *);
    int (*send_debug_prn)(void *);
    int (*write_data)(void *, const uint8_t *, size_t, const void *, size_t, int);
    /* OEM _raster2printer 0x64e1/0x64ee, 0x65e1 and 0x669a
     * reloads its original __ZL4sock separately for each socket use.
     * Optional C adapter reader: NULL retains old isolated fixtures. */
    int (*current_socket_fd)(void *);
} classic_original_raster_finish_ops;
typedef struct {
    const unsigned char *descriptor; /* exactly 0x10a8 bytes */
    int raster_fd;                    /* open(argv[6]) or stdin fd 0 */
    int socket_fd;                    /* manufacturer's static sock */
    int terminated;                   /* manufacturer global terminate_flag */
    int return_code;                  /* original local return slot -0x7bf4 */
    uint32_t *job_size;               /* original global job_size */
} classic_original_raster_finish_state;

/* OEM _WriteData() results and the close() return values are ignored.
 * The original's signed modulo-64 calculation controls the extra ESC.
 * Unlike a normal production driver, this intentionally preserves
 * unusual/error-prone manufacturer behaviors. */
int classic_original_raster_finish_job(
    const classic_original_raster_finish_ops *ops,
    const classic_original_raster_finish_state *state);
#endif
