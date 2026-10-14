/* macOS Leitz Icon 5.1.465, original _ProcessPage modes 2 and 3.
 * Direct OEM original-Mach-O call boundaries (SHA256
 * 72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13):
 * mode 3 ESC B 0x100007c38, ESC D 0x100007d6d, row 0x100007e5e;
 * mode 2 ESC B 0x10000808c, ESC D 0x10000820e,
 *        marker 0x10000866e/0x1000087b6,
 *        run/raw data 0x1000086fd/0x10000885d.
 * Leading blank-line skips are emitted before these mode branches.
 *
 * Exact row-byte algorithms are re-used from classic_process_page_modes_23;
 * this file reconstructs original separate _WriteData calls, job-size
 * increments, progress and per-row cancellation for defined normal pages.
 * This is not a completed original C++ ABI or hardware transport.
 */
#include "original_raster_process_page_modes23.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void emit(const classic_original_page_lifecycle_ops *ops,
                 const classic_oem_read_descriptor *descriptor,
                 const uint8_t *data,size_t length) {
    classic_oem_read_descriptor by_value;
    memcpy(&by_value,descriptor,sizeof by_value);
    *ops->job_size+=(uint32_t)length;
    (void)ops->write_data(ops->context,data,length,&by_value,
                          sizeof by_value,ops->socket_fd);
}

int classic_original_process_page_modes23(
    const classic_original_process_modes23_io *io,
    const classic_oem_read_descriptor *descriptor,
    const uint8_t *pixels,unsigned page_index) {
    uint32_t mode;
    memcpy(&mode,(const uint8_t *)descriptor+0x40,sizeof mode);
    if ((mode!=2u && mode!=3u) || !io->job_cache) return -2;

    const unsigned stride=descriptor->packed_bytes_per_line;
    const unsigned height=descriptor->cups_height;
    classic_mode23_page page;
    classic_mode23_begin(&page,io->job_cache,mode,stride,0);
    uint8_t *encoded=malloc((size_t)stride+11);
    if (!encoded) return -2;  /* OEM allocation failure not yet recovered */

    for (unsigned y=0;y<height;++y) {
        if ((y & 31u)==0u) {
            const int progress=(int)((100.0*(double)(float)y)/
                                     (double)(float)height);
            fprintf(stderr,"\nINFO: Printing page %d, %d%% complete...",
                    (int)(page_index+1),progress);
            fprintf(stderr,"\nATTR: job-media-progress=%d",progress);
        }

        const uint8_t *row=pixels+(size_t)y*stride;
        unsigned left=0;
        while (left<stride && row[left]==0) ++left;
        const int nonwhite=left!=stride;
        const int has_skip=nonwhite && page.pending_empty_rows!=0;
        const int has_left=nonwhite &&
            (mode==2u || left!=page.previous_left_offset);
        const unsigned previous_width=mode==2u ?
            io->job_cache->mode2_previous_width :
            io->job_cache->mode3_previous_width;
        const int first_nonblank=page.first_nonblank;
        const int reprint=page.reprint;

        const size_t n=classic_mode23_row(&page,row,encoded);
        if (n!=0) {
            /* Manufacturer emits command fragments with distinct
             * _WriteData calls. The independent row encoder supplies
             * original bytes and updates width/offset caches. */
            size_t at=0;
            if (has_skip) {
                emit(&io->output,descriptor,encoded+at,4);
                at+=4;
            }
            if (has_left) {
                emit(&io->output,descriptor,encoded+at,3);
                at+=3;
            }
            const unsigned new_width=mode==2u ?
                io->job_cache->mode2_previous_width :
                io->job_cache->mode3_previous_width;
            const int has_width=new_width!=previous_width ||
                (mode==2u && first_nonblank && reprint);
            if (has_width) {
                emit(&io->output,descriptor,encoded+at,3);
                at+=3;
            }
            if (mode==2u) {
                /* The native code sends its 0x16/0x17 selector in a
                 * separate one-byte call from the payload, regardless
                 * of whether bit-run compression was advantageous. */
                emit(&io->output,descriptor,encoded+at,1);
                ++at;
                if (at<n) emit(&io->output,descriptor,encoded+at,n-at);
            } else {
                /* Mode 3 sends 0x16 and the raw bytes together. */
                emit(&io->output,descriptor,encoded+at,n-at);
            }
        }
        /* Blank rows produce no _WriteData. OEM does not flush
         * trailing deferred blanks on page exit. */
        if (io->is_terminated &&
            io->is_terminated(io->termination_context)) break;
    }
    free(encoded);
    return 0;
}
