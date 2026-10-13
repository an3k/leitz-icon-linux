/* Independently reconstructed classic macOS raster2printer row geometry.
 * Original _ReadPage: Mach-O 0x100008e16..0x100008eb0.
 * The printable packed row stride is ceil(cupsBytesPerLine/cupsBitsPerPixel),
 * NOT ceil(cupsWidth/8). This intentionally differs from the CUPS page width.
 */
#ifndef MAC_CLASSIC_GEOMETRY_H
#define MAC_CLASSIC_GEOMETRY_H
#include "mac_classic_read_page.h"

/* Returns 0 and a packed monochrome stride in bytes if the source geometry
 * can safely be represented by the recovered original output commands.
 * The OEM filter does not validate all of these bounds; those validations
 * are intentional hardening, not manufacturer-identical error behavior.
 */
int mac_classic_packed_stride(const mac_classic_page_info *page,
                              unsigned *packed_bytes_per_line);
#endif
