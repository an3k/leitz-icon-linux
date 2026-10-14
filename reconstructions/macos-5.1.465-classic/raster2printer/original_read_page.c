/* Original _ReadPage, macOS classic 5.1.465 raster2printer Mach-O,
 * x86_64 0x100008d20..0x10000918b.
 * Preserves the original read/allocate/free order, unchecked strcpy,
 * integer divisions, diagnostic strings and zero-read dangling pointer.
 * No extra retries, bounds checks or fixed paper length.
 */
#include "original_read_page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int classic_original_read_page(classic_oem_read_descriptor *descriptor,
                               cups_page_header2_t *page_header,
                               unsigned char **pixels,
                               int *requested_bytes,
                               int *page_counter) {
    unsigned int bytes_read;
    if (!cupsRasterReadHeader2(descriptor->cups_raster,page_header))
        return 0;
    ++*page_counter;
    fprintf(stderr,"\nraster2printer.c:1049:ReadPage(): cups_page_header, media type/name = %u/'%s';",
            page_header->cupsMediaType,page_header->cupsPageSizeName);

    descriptor->cups_width=(uint16_t)page_header->cupsWidth;
    descriptor->cups_height=(uint16_t)page_header->cupsHeight;
    descriptor->cups_bytes_per_line=page_header->cupsBytesPerLine;
    descriptor->packed_bytes_per_line=
        page_header->cupsBytesPerLine/page_header->cupsBitsPerPixel +
        (page_header->cupsBytesPerLine%page_header->cupsBitsPerPixel != 0);
    descriptor->cups_bits_per_pixel=page_header->cupsBitsPerPixel;
    descriptor->cups_color_space=page_header->cupsColorSpace;
    strcpy(descriptor->cups_page_size_name,page_header->cupsPageSizeName);

    fprintf(stderr,"\nraster2printer.c:1059:ReadPage(): CUPS BYTES per line : %i",
            (int)descriptor->cups_bytes_per_line);
    fprintf(stderr,"\nraster2printer.c:1061:ReadPage(): BYTES per line : %i",
            (int)descriptor->packed_bytes_per_line);

    if (*page_counter && *pixels) free(*pixels);
    *requested_bytes=(int)(page_header->cupsBytesPerLine*page_header->cupsHeight);
    *pixels=calloc(1,(size_t)*requested_bytes);
    bytes_read=cupsRasterReadPixels(descriptor->cups_raster,*pixels,(unsigned)*requested_bytes);
    if (bytes_read==0) {
        *requested_bytes=0;
        if (*pixels) free(*pixels);
        /* OEM does not set *pixels=NULL after free. */
        return 0;
    }
    fprintf(stderr,"\nraster2printer.c:1089:ReadPage(): \n bytesRead = %i, shouldReadBytes = %i",
            (int)bytes_read,*requested_bytes);
    return 1;
}
