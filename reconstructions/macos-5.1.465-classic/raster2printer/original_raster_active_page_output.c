/* OEM macOS Leitz Icon 5.1.465 raster2printer x86_64 Mach-O:
 * _raster2printer, normal non-reprint page output, 0x100005f3b..0x1000062e5.
 * This follows the previously recovered inline read/alloc branch.
 * Original _DitherPage, _StartPage, _ProcessPage and _EndPage are separate
 * vendor functions; their invocation is mediated by analysis callbacks
 * here, not replaced by newly invented encoder or transport behavior.
 */
#include "original_raster_active_page_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int classic_original_raster_output_page(
    const classic_oem_page_output_io *io,
    classic_oem_read_descriptor *descriptor,
    classic_oem_active_page_state *page,
    unsigned *page_index,
    int has_page,int terminated,
    int *last_process_result) {

    if (terminated || !has_page) return 0;

    if (page->requested_bytes && page->page_buffer) {
        /* OEM checks cupsColorSpace==1 and its local reprint flag==0.
         * The normal path keeps that flag at its initialized value 0. */
        if (descriptor->cups_color_space == 1u) {
            unsigned quality=0;
            memcpy(&quality,(const unsigned char *)descriptor+0x4c,
                   sizeof(quality));
            io->dither_page(io->context,descriptor,page->page_buffer,
                            page->dithered_page,quality);
        }
        io->start_page(io->context,descriptor);
        const uint8_t *print_pixels=descriptor->cups_color_space==1u
                                 ? page->dithered_page : page->page_buffer;
        *last_process_result=io->process_page(io->context,descriptor,
                                               print_pixels,*page_index,0);
        /* Original EndPage receives a full 0x10a8-byte descriptor by value. */
        classic_oem_read_descriptor copy;
        memcpy(&copy,descriptor,sizeof(copy));
        io->end_page(io->context,&copy,sizeof(copy));
        fprintf(stderr,
          "\nraster2printer.c:393:raster2printer(): \n shouldResumePrintingOfLastLabel = false");
    } else {
        fprintf(stderr,"\nINFO: pageBuffer null\n");
        *last_process_result=1;
    }

    ++*page_index;
    fprintf(stderr,"\nSTATE: none\n");
    fprintf(stderr,"\nPAGE: %i 1\n",(int)*page_index);

    if (page->dithered_page) {
        free(page->dithered_page);
        page->dithered_page=NULL;
    }
    if (page->page_buffer) {
        free(page->page_buffer);
        page->page_buffer=NULL;
    }
    /* OEM repeats only if _ProcessPage returned 0 and a page was present. */
    return *last_process_result==0 && has_page;
}

/* Unreachable original reprint-true call sequence at OEM
 * _raster2printer 0x100006135..0x1000061d6.
 *
 * Earlier original instructions set shouldResumePrintingOfLastLabel to
 * false at 0x100005603, with no subsequent assignment in the entire
 * raster page-read/output loop (audited from 0x1000055ce..0x1000062e5).
 * Therefore do NOT use this dormant branch to introduce extra printing
 * in the normal CUPS job. Its original call effects are preserved here
 * for completeness, separately from the reachable page algorithm.
 *
 * On entry the OEM zeros descriptor offset +0x24, selects the already
 * allocated dither output only for cupsColorSpace==1, calls
 * _ProcessPage(descriptor, pixels, index, true) and then _EndPage with
 * a full 0x10a8-byte descriptor COPY. There is NO _StartPage or
 * _DitherPage call in this reprint-only sequence. Its surrounding OEM
 * loop owns eventual buffer cleanup and PAGE diagnostics.
 */
int classic_original_raster_reprint_page(
    const classic_oem_page_output_io *io,
    classic_oem_read_descriptor *descriptor,
    const classic_oem_active_page_state *page,
    unsigned page_index) {
    /* OEM reprint caller 0x100006135 clears the very same mode-3
     * width field consumed at _ProcessPage 0x100007c78. */
    descriptor->mode3_previous_width=0;
    const uint8_t *pixels=descriptor->cups_color_space==1u
                           ? page->dithered_page : page->page_buffer;
    int result=io->process_page(io->context,descriptor,pixels,page_index,1);
    classic_oem_read_descriptor copy;
    memcpy(&copy,descriptor,sizeof copy);
    io->end_page(io->context,&copy,sizeof copy);
    return result;
}
