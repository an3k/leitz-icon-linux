#include "mac_classic_raster_diagnostics.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned page, rows_read; } fake_source;
static int next_header(void *context, mac_classic_raster_diagnostic_header *h) {
  fake_source *s = context;
  if (s->page++ == 2) return 0;
  h->cups_width = 16;
  h->num_copies = 2;
  h->cups_height = 3;
  h->cups_bytes_per_line = 2;
  return 1;
}
static size_t next_pixels(void *context, uint8_t *buf, size_t bytes) {
  fake_source *s=context;
  ++s->rows_read;
  memset(buf,0xa5,bytes);
  return bytes;
}
int main(void) {
  mac_classic_raster_diagnostic_header h={0};
  h.cups_bytes_per_line=120;h.cups_bits_per_color=8;
  h.cups_bits_per_pixel=24;h.cups_color_order=0;
  h.cups_width=960;h.cups_height=300;h.hw_resolution[0]=300;
  h.hw_resolution[1]=600;
  memcpy(h.cups_page_size_name,"USER",5);
  FILE *f=tmpfile(); assert(f);
  assert(mac_classic_print_page_debug_info(f,&h)==0);
  assert(fflush(f)==0);assert(fseek(f,0,SEEK_SET)==0);
  char buf[1024];size_t n=fread(buf,1,sizeof buf-1,f);buf[n]=0;
  assert(strstr(buf,"cupsBytesPerLine = 120")!=NULL);
  assert(strstr(buf,"cupsPageSizeName='USER'")!=NULL);
  assert(strstr(buf,"HWResolution=[300, 600]")!=NULL);
  fclose(f);
  fake_source source={0};
  mac_classic_raster_diagnostic_source api={next_header,next_pixels};
  unsigned pages=0;
  assert(mac_classic_test_raster(&api,&source,NULL,&pages)==0);
  assert(pages==2 && source.rows_read==6);
  return 0;
}
