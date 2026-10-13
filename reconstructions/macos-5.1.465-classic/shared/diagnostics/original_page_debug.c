/* Functional C reconstruction of both original _PrintPageDebugInfo
 * functions, directly disassembled from macOS 5.1.465:
 * raster2printer 0x100003e50..0x100004419
 * command2leitz  0x100004480..0x100004a49
 * OEM format strings, pathname basename and line numbers are preserved.
 * This function is dormant in the standard printing/command entry paths.
 */
#include <cups/raster.h>
#include <stdio.h>
#include <string.h>

void PrintPageDebugInfo(cups_page_header2_t *header) {
    static const char original_file[] =
        "/Volumes/hfsbuild/leitzicon-mac/PinterDriver/SampleRaster&Dymo/Leitz/raster_c_functions.c";
    const char *file;
    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr, "\n%s:%d:%s(): cupsBytesPerLine = %d",
            file, 385, "PrintPageDebugInfo", (int)header->cupsBytesPerLine);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr, "\n%s:%d:%s(): cupsBitsPerColor = %d",
            file, 386, "PrintPageDebugInfo", (int)header->cupsBitsPerColor);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr, "\n%s:%d:%s(): cupsBitsPerPixel = %d",
            file, 387, "PrintPageDebugInfo", (int)header->cupsBitsPerPixel);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr, "\n%s:%d:%s(): cupsColorOrder = %d",
            file, 388, "PrintPageDebugInfo", (int)header->cupsColorOrder);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr,
            "\n%s:%d:%s(): cupsWidth=%u; .cupsHeight = %u; .cupsBorderlessScalingFactor=%2.5f",
            file, 389, "PrintPageDebugInfo", header->cupsWidth,
            header->cupsHeight, (double)header->cupsBorderlessScalingFactor);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr,
            "\n%s:%d:%s(): HWResolution=[%u, %u]; .ImagingBoundingBox=[%u, %u, %u, %u]; .Margins=[%u, %u]; .PageSize=[%u, %u]; .cupsImagingBBox=[%2.5f, %2.5f, %2.5f, %2.5f]; cupsPageSize=[%2.5f, %2.5f]",
            file, 391, "PrintPageDebugInfo",
            header->HWResolution[0], header->HWResolution[1],
            header->ImagingBoundingBox[0], header->ImagingBoundingBox[1],
            header->ImagingBoundingBox[2], header->ImagingBoundingBox[3],
            header->Margins[0], header->Margins[1],
            header->PageSize[0], header->PageSize[1],
            (double)header->cupsImagingBBox[0],
            (double)header->cupsImagingBBox[1],
            (double)header->cupsImagingBBox[2],
            (double)header->cupsImagingBBox[3],
            (double)header->cupsPageSize[0],
            (double)header->cupsPageSize[1]);

    if (strrchr(original_file, '/'))
        file = strrchr(original_file, '/') + 1;
    else
        file = original_file;
    fprintf(stderr, "\n%s:%d:%s(): cupsPageSizeName='%s';",
            file, 392, "PrintPageDebugInfo", header->cupsPageSizeName);
}
