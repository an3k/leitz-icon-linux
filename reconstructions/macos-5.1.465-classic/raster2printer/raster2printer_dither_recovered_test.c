#include "raster2printer_dither_recovered.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
int main(void) {
  uint8_t rgb[27],out[2];
  memset(rgb,0,sizeof rgb); memset(out,0,sizeof out);
  classic_raster_dither_rgb24_recovered(0,rgb,1,27,out);
  assert(out[0]==0xff && out[1]==0x80);
  memset(rgb,255,sizeof rgb); memset(out,0xff,sizeof out);
  classic_raster_dither_rgb24_recovered(0,rgb,1,27,out);
  assert(out[0]==0 && out[1]==0);
  memset(rgb,0,sizeof rgb);
  for(unsigned mode=0;mode<15;++mode) {
    if(mode==7) continue;
    memset(out,0,sizeof out);
    classic_raster_dither_rgb24_recovered(mode,rgb,1,27,out);
    assert(out[0]==0xff && out[1]==0x80);
  }
  puts("classic macOS recovered RGB24 core checks passed");
  return 0;
}
