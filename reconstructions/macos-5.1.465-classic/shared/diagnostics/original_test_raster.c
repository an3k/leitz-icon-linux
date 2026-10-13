/* Functional C reconstruction of the manufacturer _test_raster helper.
 * Direct disassembly evidence:
 *   classic raster2printer 0x100003c60..0x100003e40
 *   classic command2leitz 0x100004290..0x100004470
 * Original x86_64 Mach-O binaries: Leitz 5.1.465.
 * Both originals contain the same control flow, argument choices and
 * diagnostic string. These dormant functions are not activated by the
 * ordinary print or command entry paths.
 *
 * No added allocation check, partial-read rejection, or retry: those
 * would change the original behavior. The original hard-coded build
 * pathname and its diagnostics are intentionally preserved.
 */
#include <cups/raster.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_raster(void) {
    static const char original_file[] =
        "/Volumes/hfsbuild/leitzicon-mac/PinterDriver/SampleRaster&Dymo/Leitz/raster_c_functions.c";
    cups_raster_t *raster = cupsRasterOpen(0, (cups_mode_t)0);
    cups_page_header2_t page_header;
    int page = 0;

    while (cupsRasterReadHeader2(raster, &page_header) != 0) {
        const char *basename;
        unsigned char *line;
        unsigned row = 0;
        ++page;
        if (strrchr(original_file, '/'))
            basename = strrchr(original_file, '/') + 1;
        else
            basename = original_file;
        fprintf(stderr, "\n%s:%d:%s(): PAGE: %d %d",
                basename, 358, "test_raster", page, (int)page_header.NumCopies);

        line = malloc(page_header.cupsBytesPerLine);
        while (row < page_header.cupsHeight) {
            if (cupsRasterReadPixels(raster, line, page_header.cupsBytesPerLine) == 0)
                break;
            ++row;
        }
        if (line != NULL) {
            free(line);
            line = NULL;
        }
    }
    cupsRasterClose(raster);
}
