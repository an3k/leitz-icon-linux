#ifndef ORIGINAL_RASTER_PROCESS_PAGE_MODES01_H
#define ORIGINAL_RASTER_PROCESS_PAGE_MODES01_H
#include "original_raster_page_lifecycle.h"

/* Original macOS Classic 5.1.465 _ProcessPage normal mode 0/1 control
 * recovered from native 0x100007560..0x1000089a6. Existing isolated OEM
 * per-row encoders are reused without altering their algorithms.
 *
 * This is a newly authored callback ABI, NOT the manufacturer C++ ABI.
 * Caller must provide readable row data through (height * stride + 4)
 * bytes: the original sends four bytes beyond the final raster row,
 * an undefined original memory region for exact-sized mono buffers.
 * Never interpret this extra range as authenticated raster pixels.
 */
typedef struct {
    classic_original_page_lifecycle_ops output;
    int (*is_terminated)(void *);
    void *termination_context;
} classic_original_process_modes01_io;

/* Supported only for original descriptor optimization mode 0 or 1.
 * Returns OEM 0 for these paths, including an ignored WriteData failure
 * and cancellation after a row. Returns -2 ONLY for unsupported modes
 * (reconstruction-only sentinel; no OEM analogue). Does not call
 * _StartPage/_EndPage; the existing OEM page coordinator owns them.
 */
int classic_original_process_page_modes01(
    const classic_original_process_modes01_io *io,
    const classic_oem_read_descriptor *descriptor,
    const unsigned char *pixels,
    unsigned page_index);
#endif
