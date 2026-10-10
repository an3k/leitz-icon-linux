/* All assertions target command structure, not physical device behaviour. */
#include "../src/leitz_native_reference.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test(unsigned dpi) {
  FILE *f = tmpfile();
  assert(f);
  leitz_native_reference p;
  assert(leitz_native_begin(&p, f, 16, 4, dpi, 2) == 0);
  const uint8_t white[2] = {0, 0};
  const uint8_t ink[2] = {0x81, 0x42};
  assert(leitz_native_row(&p, white, 2) == 0);
  assert(leitz_native_row(&p, ink, 2) == 0);
  assert(leitz_native_row(&p, white, 2) == 0);
  assert(leitz_native_row(&p, white, 2) == 0);
  assert(leitz_native_end(&p) == 0);
  assert(leitz_native_end(&p) == -1);
  assert(leitz_native_row(&p, ink, 2) == -1);
  assert(fflush(f) == 0);
  assert(fseek(f, 0, SEEK_END) == 0);
  long len = ftell(f);
  assert(len == 4 + 120 + 11 + 4 + 6 + 4 + 4);
  rewind(f);
  uint8_t got[200] = {0};
  assert(fread(got, 1, (size_t)len, f) == (size_t)len);
  assert(got[0] == 0x1b && got[1] == '@');
  assert(got[2] == 0x1b && got[3] == (dpi == 600 ? 'i' : 'h'));
  for (unsigned i = 4; i < 124; i++) assert(got[i] == 0x1b);
  const uint8_t setup[] = {0x1b,'L',0,2,0x1b,'w',0,16,0x1b,'B',0};
  assert(memcmp(got+124, setup, sizeof setup) == 0);
  const uint8_t body[] = {0x1b,'f',0,1,0x1b,'D',2,0x16,0x81,0x42,0x1b,'f',0,2,0x1b,'G',0x1b,'E'};
  assert(memcmp(got+135, body, sizeof body) == 0);
  fclose(f);
}
static void test_rejection(void) {
  FILE *f = tmpfile();
  assert(f);
  leitz_native_reference p;
  assert(leitz_native_begin(&p, f, 961, 10, 600, 10) == -1);
  assert(leitz_native_begin(&p, f, 16, 2, 400, 1) == -1);
  assert(leitz_native_begin(&p, f, 16, 2, 600, 1) == 0);
  assert(leitz_native_end(&p) == -1);
  uint8_t pixels[2] = {0, 0};
  assert(leitz_native_row(&p, pixels, 1) == -1);
  assert(leitz_native_row(&p, pixels, 2) == 0);
  assert(leitz_native_row(&p, pixels, 2) == 0);
  assert(leitz_native_row(&p, pixels, 2) == -1);
  assert(leitz_native_end(&p) == 0);
  fclose(f);
}
int main(void) {
  test(300);test(600);test_rejection();
  puts("PASS vendor-framed raster commands at 300 and 600 dpi (offline)");
  return 0;
}
