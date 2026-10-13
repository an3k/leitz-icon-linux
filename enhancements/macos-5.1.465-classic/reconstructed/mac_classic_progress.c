#include "mac_classic_progress.h"

/* _ProcessPage emits two stderr records when (row & 31) == 0.
 * The source expression uses float intermediate values and converts the
 * percentage to int (truncation toward zero), not a rounded percentage.
 * The algebraically equivalent integer form avoids rounding up. */
int mac_classic_raster_progress(unsigned row, unsigned rows, unsigned *percent) {
  if (!rows || row >= rows || !percent) return -1;
  if ((row & 31u) != 0) return 0;
  *percent = (unsigned)((100ull * row) / rows);
  return 1;
}
