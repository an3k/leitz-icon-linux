/* Original _raster2printer case-insensitive PPD choice comparisons,
 * observed at Mach-O 0x100004dbd..0x100005098.
 * The public lookup is an ANALYSIS accessor; its invalid-name sentinel is
 * not a claim about the original executable's fallback control flow.
 */
#include "raster2printer_quality_recovered.h"
#include <strings.h>
#include <stddef.h>

static const char *const quality_names[15] = {
  "Bayer8x8",
  "Bayer8x8SuperCell1",
  "Bayer8x8SuperCell2",
  "SuperCell1",
  "SuperCell2",
  "SuperCell3",
  "CUPSThresold64x64",
  "Draft",
  "SuperCell",
  "Bayer4x4",
  "Bayer14x14",
  "Clustered8x8",
  "Dispersed8x8",
  "Nasik4x4",
  "Dithering3x3"
};

int classic_raster_quality_selector(const char *name) {
  unsigned i;
  if (!name) return -1;
  for (i=0; i<15; ++i)
    if (strcasecmp(name,quality_names[i])==0) return (int)i;
  return -1;
}

/* The OEM Draft PPD choice selects 1-bit input and does not enter the RGB24
 * dither path. Its original table pointer is uninitialized at entry: the
 * explicit null entry below is ONLY a data-inventory placeholder and MUST
 * NOT be used to infer the executable's undefined behavior for mode 7.
 */
const classic_raster_halftone_table classic_raster_quality_tables[15] = {
  {matrix_bayer8x8, 8, 64.0}, /* Bayer8x8 */
  {matrix_reorderedBayer, 10, 64.0}, /* Bayer8x8SuperCell1 */
  {matrix_reorderedBayer, 12, 64.0}, /* Bayer8x8SuperCell2 */
  {matrix_orderedSuperCell, 18, 256.0}, /* SuperCell1 */
  {matrix_reorderedBayer3, 12, 70.0}, /* SuperCell2 */
  {matrix_supercell1, 6, 20.0}, /* SuperCell3 */
  {matrix_CUPSthreshold, 64, 255.0}, /* CUPSThresold64x64 */
  {NULL, 0, 0.0}, /* Draft */
  {matrix_supercell, 8, 31.0}, /* SuperCell */
  {matrix_bayer4x4, 4, 16.0}, /* Bayer4x4 */
  {matrix_bayer14x14, 14, 255.0}, /* Bayer14x14 */
  {matrix_clustered8x8, 8, 40.0}, /* Clustered8x8 */
  {matrix_dispersed8x8, 8, 45.0}, /* Dispersed8x8 */
  {matrix_nasik4x4, 4, 16.0}, /* Nasik4x4 */
  {matrix_dithering3x3, 3, 8.0} /* Dithering3x3 */
};
