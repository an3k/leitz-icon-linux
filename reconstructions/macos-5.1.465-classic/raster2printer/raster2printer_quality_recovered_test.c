#include "raster2printer_quality_recovered.h"
#include <assert.h>
#include <stddef.h>
#include <strings.h>
#include <stdio.h>
static const char *const names[15] = {
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
int main(void) {
  unsigned i;
  for (i=0;i<15;i++) {
    assert(classic_raster_quality_selector(names[i])==(int)i);
    assert(classic_raster_quality_tables[i].matrix || i==7);
    assert(classic_raster_quality_tables[i].dimension || i==7);
  }
  assert(classic_raster_quality_selector("draft")==7);
  assert(classic_raster_quality_selector("BAYER8X8")==0);
  assert(classic_raster_quality_selector("not-an-oem-choice")==-1);
  assert(classic_raster_quality_selector(NULL)==-1);
  assert(matrix_bayer8x8[0]==0);
  assert(matrix_bayer4x4[0]==1);
  assert(matrix_dithering3x3[0]==3);
  puts("classic macOS halftone data/PPD selection checks passed");
  return 0;
}
