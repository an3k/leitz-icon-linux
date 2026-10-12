/* Reconstructed fragments from classic macOS raster2printer; not complete. */
#ifndef MAC_CLASSIC_RASTER_BOUNDARIES_H
#define MAC_CLASSIC_RASTER_BOUNDARIES_H
#include <stddef.h>
#include <stdint.h>

/* _StartDocument (0x100006890): 120 ESC padding bytes then ESC @;
 * optional ESC i when high_resolution != 0. */
size_t mac_classic_start_document(int high_resolution, uint8_t out[124]);

/* _EndPage (0x1000089b0): emit ESC G and, if cutEveryLabel != 0, ESC E. */
size_t mac_classic_end_page(int cut_every_label, uint8_t out[4]);

/* _EndDocument (0x100008ac0): if cutEveryLabel==0 and !terminateFlag,
 * emit ESC E, followed unconditionally by ESC @ ESC Z. */
size_t mac_classic_end_document(int cut_every_label, int terminate_flag, uint8_t out[6]);

#endif
