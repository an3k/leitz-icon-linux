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

/* Original raster2printer 5.1.465 `nm -nm` enumerates ALL 13
 * tables as 32-bit entries in writable __DATA,__data, with precisely
 * the following file-local C++ labels at VAs 0x10000c240..0x100011483.
 * Their 13 complete byte-identical numeric payloads are already
 * reconstructed in raster2printer_matrices_recovered.c. Do not duplicate
 * or regenerate them. These GNU/Clang assembler labels provide the
 * original named data symbols in native Apple C builds. They do NOT
 * imply that C linkage or the local-visibility bit exactly reproduces
 * the original C++ internal linkage.
 */
#if defined(__APPLE__)
#define CLASSIC_OEM_MATRIX(name,count,original) \
    extern int32_t name[count] __asm__(original)
#else
#define CLASSIC_OEM_MATRIX(name,count,original) \
    extern int32_t name[count]
#undef CLASSIC_OEM_MATRIX
#endif

CLASSIC_OEM_MATRIX(matrix_supercell, 64, "__ZL9supercell");
CLASSIC_OEM_MATRIX(matrix_bayer4x4, 16, "__ZL8bayer4x4");
CLASSIC_OEM_MATRIX(matrix_bayer8x8, 64, "__ZL8bayer8x8");
CLASSIC_OEM_MATRIX(matrix_CUPSthreshold, 4096, "__ZL13CUPSthreshold");
CLASSIC_OEM_MATRIX(matrix_orderedSuperCell, 324, "__ZL16orderedSuperCell");
CLASSIC_OEM_MATRIX(matrix_reorderedBayer3, 144, "__ZL15reorderedBayer3");
CLASSIC_OEM_MATRIX(matrix_supercell1, 36, "__ZL10supercell1");
CLASSIC_OEM_MATRIX(matrix_reorderedBayer, 144, "__ZL14reorderedBayer");
CLASSIC_OEM_MATRIX(matrix_bayer14x14, 196, "__ZL10bayer14x14");
CLASSIC_OEM_MATRIX(matrix_clustered8x8, 64, "__ZL12clustered8x8");
CLASSIC_OEM_MATRIX(matrix_dispersed8x8, 64, "__ZL12dispersed8x8");
CLASSIC_OEM_MATRIX(matrix_nasik4x4, 16, "__ZL8nasik4x4");
CLASSIC_OEM_MATRIX(matrix_dithering3x3, 9, "__ZL12dithering3x3");
#endif
