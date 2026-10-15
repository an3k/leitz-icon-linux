#ifndef ORIGINAL_RASTER_PROCESS_PAGE_MODES23_H
#define ORIGINAL_RASTER_PROCESS_PAGE_MODES23_H

#include "original_raster_page_lifecycle.h"
#include "classic_process_page_modes_23.h"

/* OEM _ProcessPage static local persisted across page invocations:
 * native C++ mangled label __ZZ11ProcessPageE26bytesPrintedInPreviousLine.
 * Its memory width and read/write sites are verified in original Mach-O.
 */
extern uint32_t classic_original_mode2_static_width;

/* Native macOS 5.1.465 ProcessPage: modes 2/3 and the verified
 * mode-2 reprint-first-row width-command exception.
 * This bridge preserves the independently reconstructed row encoder.
 * Original CUPS descriptor, shared width cache and globals are supplied
 * by the caller. Callbacks are new C ABI, not the original Mach-O ABI.
 */
typedef struct {
    classic_original_page_lifecycle_ops output;
    classic_mode23_job_cache *job_cache;  /* retained across pages */
    int (*is_terminated)(void *);
    void *termination_context;
    unsigned reprint; /* Fourth OEM ProcessPage argument; 0 in normal job */
} classic_original_process_modes23_io;

/* Returns 0 for original defined mode-2/3 paths. -2 denotes only a
 * reconstruction-unsupported/failed-allocation case; not an OEM status.
 * _StartPage and _EndPage belong to the surrounding page coordinator. */
int classic_original_process_page_modes23(
    const classic_original_process_modes23_io *io,
    const classic_oem_read_descriptor *descriptor,
    const uint8_t *pixels,
    unsigned page_index);

#endif
