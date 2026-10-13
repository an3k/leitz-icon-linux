#ifndef CLASSIC_RASTER_QUALITY_H
#define CLASSIC_RASTER_QUALITY_H
#include <stdint.h>

/* Reference data extracted from the original 5.1.465 classic Mach-O.
 * These are reconstructed declarations, not vendor-supplied C source. */
typedef struct {
  const int32_t *matrix;
  unsigned dimension;
  double divisor;
} classic_raster_halftone_table;

/* 0..14: exact selector indices observed in original option comparisons.
 * -1: name outside the 15 observed choices; not a vendor error mode. */
int classic_raster_quality_selector(const char *name);
extern const classic_raster_halftone_table classic_raster_quality_tables[15];

extern const int32_t matrix_supercell[64];
extern const int32_t matrix_bayer4x4[16];
extern const int32_t matrix_bayer8x8[64];
extern const int32_t matrix_CUPSthreshold[4096];
extern const int32_t matrix_orderedSuperCell[324];
extern const int32_t matrix_reorderedBayer3[144];
extern const int32_t matrix_supercell1[36];
extern const int32_t matrix_reorderedBayer[144];
extern const int32_t matrix_bayer14x14[196];
extern const int32_t matrix_clustered8x8[64];
extern const int32_t matrix_dispersed8x8[64];
extern const int32_t matrix_nasik4x4[16];
extern const int32_t matrix_dithering3x3[9];
#endif
