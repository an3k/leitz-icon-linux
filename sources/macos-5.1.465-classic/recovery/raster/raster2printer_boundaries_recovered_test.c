#include "raster2printer_boundaries_recovered.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
  uint8_t output[124];
  size_t n=classic_raster_start_document(0,output);
  assert(n==122);
  for (size_t i=0;i<121;++i) assert(output[i]==0x1b);
  assert(output[121]=='@');
  n=classic_raster_start_document(1,output);
  assert(n==124 && output[122]==0x1b && output[123]=='i');
  n=classic_raster_start_page(704,100,0,output);
  assert(n==11 && memcmp(output,"\033L\000\202\033w\002\300\033B\000",11)==0);
  n=classic_raster_start_page(704,100,1,output);
  assert(n==11 && output[2]==0 && output[3]==80);
  n=classic_raster_end_page(0,output);
  assert(n==2 && !memcmp(output,"\033G",n));
  n=classic_raster_end_page(1,output);
  assert(n==4 && !memcmp(output,"\033G\033E",n));
  n=classic_raster_end_document(0,0,output);
  assert(n==6 && !memcmp(output,"\033E\033@\033Z",n));
  n=classic_raster_end_document(0,1,output);
  assert(n==4 && !memcmp(output,"\033@\033Z",n));
  n=classic_raster_end_document(1,0,output);
  assert(n==4 && !memcmp(output,"\033@\033Z",n));
  FILE *f=tmpfile();assert(f);assert(fputs("x",f)>=0);
  classic_raster_flush_data_streams(f,NULL);
  assert(fclose(f)==0);
  puts("raster2printer boundary checks passed");
  return 0;
}
