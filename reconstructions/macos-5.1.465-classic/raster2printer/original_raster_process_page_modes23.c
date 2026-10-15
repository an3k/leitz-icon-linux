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

/* Actual original _ProcessPage file-local static C++ state:
 * __ZZ11ProcessPageE26bytesPrintedInPreviousLine
 * Native original raster2printer __DATA,__bss at 0x100011cdc.
 * OEM 0x10000815f loads this 32-bit value for mode-2 ESC D test;
 * OEM 0x10000821f stores the new trimmed width ONLY after sending
 * ESC D. The initial zero is process-static, not page-local.
 * C reconstruction exposes it across source modules using the exact
 * Apple Mach-O name, without claiming C++ private linkage identity.
 */
#if defined(__APPLE__)
uint32_t classic_original_mode2_static_width
    __asm__("__ZZ11ProcessPageE26bytesPrintedInPreviousLine");
#else
uint32_t classic_original_mode2_static_width;
#endif
_Static_assert(sizeof(classic_original_mode2_static_width)==4,
               "OEM ProcessPage static-width 32-bit type");

static void emit(const classic_original_page_lifecycle_ops *ops,
                 const classic_oem_read_descriptor *descriptor,
                 const uint8_t *data,size_t length) {
    classic_oem_read_descriptor by_value;
    memcpy(&by_value,descriptor,sizeof by_value);
    *ops->job_size+=(uint32_t)length;
    (void)ops->write_data(ops->context,data,length,&by_value,
                          sizeof by_value,ops->socket_fd);
}

/* Original OEM mode-2 bit-run initializer is:
 *   0x10000825b: run_byte = 0xff (sentinel)
 *   0x100008262: previous_bit = 0
 * At 0x1000083e5..0x100008414 a DIFFERENT first bit causes the
 * 0xff diagnostic. The current run is then reset at 0x1000084f7,
 * and subsequent runs flush on 0x7f before ever reaching 0xff.
 * Thus only the very first bit of a cropped nonblank row can log.
 * This is not a trace of every completed 128-one-bit RLE run.
 */
static void report_original_mode2_first_bit(const uint8_t *row,
                                             unsigned length) {
    if(length!=0 && (row[0]&0x80u)!=0)
        fprintf(stderr,
            "\nraster2printer.c:860:ProcessPage(): first bit/byte. do not send 255");
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
    /* OEM 0x10000815f..0x10000824d: for mode 2, the first
     * nonblank reprint row must send ESC D even for cached width.
     * The existing row encoder already implements that condition. */
    classic_mode23_begin(&page,io->job_cache,mode,stride,io->reprint);
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

        if(nonwhite && mode==2u) {
            unsigned right=stride;
            while(right>left && row[right-1]==0) --right;
            report_original_mode2_first_bit(row+left,right-left);
        }
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
                /* Original width cache is updated only by mode 2,
                 * immediately after the 3-byte ESC D _WriteData,
                 * not by blank rows, mode 3 or a suppressed width. */
                if (mode==2u)
                    classic_original_mode2_static_width=new_width;
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
