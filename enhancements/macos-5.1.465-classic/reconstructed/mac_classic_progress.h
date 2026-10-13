/* Classic macOS 5.1.465 raster2printer _ProcessPage progress calculation.
 * Original x86-64: 0x1000077a6..0x1000078a9. Offline status only;
 * this never denotes mechanical completion at the printer. */
#ifndef MAC_CLASSIC_PROGRESS_H
#define MAC_CLASSIC_PROGRESS_H

/* Returns 1 when a progress report is emitted by the original row loop,
 * 0 otherwise, and -1 on invalid geometry/pointer. Pages use zero-based
 * row indices. Original reports at row 0 and every subsequent 32 rows. */
int mac_classic_raster_progress(unsigned row, unsigned rows, unsigned *percent);
#endif
