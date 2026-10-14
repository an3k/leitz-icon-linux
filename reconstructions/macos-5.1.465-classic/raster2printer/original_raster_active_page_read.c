/* OEM macOS Leitz Icon 5.1.465 raster2printer x86_64 Mach-O:
 * _raster2printer inline page reader 0x1000056c3..0x100005f22.
 * Distinct from its separately exported _ReadPage at 0x100008d20:
 * this path uses malloc(), retains the buffer on zero pixel result,
 * and allocates a second full-size dither buffer on a positive result.
 * The original also calls cupsRasterReadPixels once if malloc failed.
 * This reconstruction deliberately adds no null-ptr retry/check.
 */
#include "original_raster_active_page_read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int classic_original_raster_active_page_read(
    classic_oem_read_descriptor *descriptor,
    cups_page_header2_t *header,
    classic_oem_active_page_state *state) {
    if (cupsRasterReadHeader2(descriptor->cups_raster,header)==0) {
        fprintf(stderr,
            "\nraster2printer.c:346:raster2printer(): No more available page : %i\n",
            state->requested_bytes);
        return 0;
    }

    ++state->page_number;
    fprintf(stderr,
       "\nraster2printer.c:300:raster2printer(): cups_page_header, media type/name = %u/'%s';",
       header->cupsMediaType,header->cupsPageSizeName);

    descriptor->cups_width=(uint16_t)header->cupsWidth;
    descriptor->cups_height=(uint16_t)header->cupsHeight;
    descriptor->cups_bytes_per_line=header->cupsBytesPerLine;
    /* OEM active inline reader 0x1000057eb divl (unsigned quotient)
     * and 0x100005800 idivl (signed remainder): the exact same
     * original arithmetic as the separate exported _ReadPage.
     * Reuse its already tested recovered rule, not an unsigned ceil. */
    descriptor->packed_bytes_per_line=classic_oem_read_page_packed_stride(
        header->cupsBytesPerLine,header->cupsBitsPerPixel);
    descriptor->cups_bits_per_pixel=header->cupsBitsPerPixel;
    descriptor->cups_color_space=header->cupsColorSpace;
    strcpy(descriptor->cups_page_size_name,header->cupsPageSizeName);

    fprintf(stderr,"\nraster2printer.c:310:raster2printer(): CupsPageSizeName : %s",
            descriptor->cups_page_size_name);
    fprintf(stderr,"\nraster2printer.c:312:raster2printer(): CUPS BYTES per line : %i\n",
            (int)descriptor->cups_bytes_per_line);
    fprintf(stderr,"\nraster2printer.c:314:raster2printer(): BYTES per line : %i\n",
            (int)descriptor->packed_bytes_per_line);

    /* OEM signed 32-bit multiplication followed by sign extension to size_t. */
    state->requested_bytes=(int32_t)(header->cupsBytesPerLine*header->cupsHeight);
    state->page_buffer=malloc((size_t)(int64_t)state->requested_bytes);
    if (state->page_buffer) {
        fprintf(stderr,
          "\nraster2printer.c:322:raster2printer(): SUCCESS malloc pageBuffer : %i\n",
          state->requested_bytes);
    } else {
        fprintf(stderr,
          "\nraster2printer.c:326:raster2printer(): Failure malloc pageBuffer : %i\n",
          state->requested_bytes);
    }
    /* The original does not short-circuit a failed malloc(). */
    state->received_bytes=cupsRasterReadPixels(descriptor->cups_raster,
               state->page_buffer,(unsigned)state->requested_bytes);
    if (state->received_bytes==0) {
        state->requested_bytes=0;
        return 0;
    }
    fprintf(stderr,
       "\nraster2printer.c:340:raster2printer():  bytesRead = %i, shouldReadBytes = %i\n",
       (int)state->received_bytes,state->requested_bytes);

    /* OEM dither allocation size is bytes-per-line * 16-bit height;
     * NOT the packed monochrome row size and NOT a fixed label length. */
    uint32_t size=header->cupsBytesPerLine*(uint32_t)descriptor->cups_height;
    state->dithered_page=malloc((size_t)size);
    if (state->dithered_page) {
        fprintf(stderr,
          "\nraster2printer.c:356:raster2printer(): SUCCESS malloc ditheredPage : %i\n",
          state->requested_bytes);
    } else {
        fprintf(stderr,
          "\nraster2printer.c:360:raster2printer(): Failure malloc ditheredPage : %i\n",
          state->requested_bytes);
    }
    return 1;
}
