/* Original classic macOS raster2printer _ProcessPage control for the
 * OEM modes 0 and 1. Native machine-code source: SHA-256
 * 72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13
 * 0x100007560..0x1000078eb (setup, ESC D/SYN, progress and row loop),
 * 0x1000078eb..0x100007a74 (deferred white lines),
 * 0x10000889b..0x1000089a6 (raw-row send, termination, return).
 *
 * This is a C reconstruction component, not a native Mach-O replacement.
 * Manufacturer-mode-0/1 row bytes come from the already reconstructed
 * classic_process_page_modes_01.c. OEM WriteData failures are ignored.
 */
#include "original_raster_process_page_modes01.h"
#include "classic_process_page_modes_01.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void send_original(const classic_original_page_lifecycle_ops *ops,
                          const classic_oem_read_descriptor *descriptor,
                          const uint8_t *bytes,size_t length) {
    classic_oem_read_descriptor by_value;
    memcpy(&by_value,descriptor,sizeof by_value);
    *ops->job_size+=(uint32_t)length;
    (void)ops->write_data(ops->context,bytes,length,&by_value,
                          sizeof by_value,
                          ops->current_socket_fd ?
                          ops->current_socket_fd(ops->context) :
                          ops->socket_fd);
}

int classic_original_process_page_modes01(
    const classic_original_process_modes01_io *io,
    const classic_oem_read_descriptor *descriptor,
    const unsigned char *pixels,
    unsigned page_index) {
    uint32_t mode;
    memcpy(&mode,(const unsigned char *)descriptor+0x40,sizeof mode);
    if (mode>1u) return -2; /* reconstruction-only unsupported mode */

    classic_mode01_state row_state;
    uint8_t preamble[4];
    const unsigned stride=descriptor->packed_bytes_per_line;
    const unsigned height=descriptor->cups_height;
    classic_mode01_begin(&row_state,mode,stride,preamble);
    send_original(&io->output,descriptor,preamble,sizeof preamble);

    /* Additional 4 input bytes are intentionally consumed by the OEM
     * raw-row path; caller must make that region readable. */
    uint8_t *encoded=malloc((size_t)stride+8);
    if (!encoded) return -2; /* unknown OEM allocation failure semantics */

    for(unsigned y=0;y<height;++y) {
        if ((y & 31u)==0u) {
            /* OEM converts integer y and 16-bit height to floating-point,
             * multiplies by 100 and truncates to int, only every 32 rows. */
            const int progress=(int)((100.0*(double)(float)y)/
                                     (double)(float)height);
            fprintf(stderr,"\nINFO: Printing page %d, %d%% complete...",
                    (int)(page_index+1),progress);
            fprintf(stderr,"\nATTR: job-media-progress=%d",progress);
        }
        const uint16_t pending_before=row_state.pending_empty_rows;
        const uint8_t *row=pixels+(size_t)y*stride;
        const size_t bytes=classic_mode01_row(&row_state,row,encoded);
        if (bytes) {
            if (mode==1u && pending_before!=0u) {
                /* Original sends ESC f separately, then the raw row. */
                send_original(&io->output,descriptor,encoded,4);
                send_original(&io->output,descriptor,encoded+4,bytes-4);
            } else {
                send_original(&io->output,descriptor,encoded,bytes);
            }
        }
        /* OEM checks SIGTERM AFTER every row, including final/blank. */
        if (io->is_terminated &&
            io->is_terminated(io->termination_context)) break;
    }
    free(encoded);
    return 0; /* OEM local result initialized 0, no active update */
}
