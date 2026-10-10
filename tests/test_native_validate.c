#include "../src/leitz_native_reference.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void verify(unsigned dpi) {
  FILE *f = tmpfile();assert(f);
  leitz_native_reference x;
  assert(!leitz_native_begin(&x,f,16,5,dpi,3));
  const uint8_t zero[2]={0,0}, one[2]={0xA5,0x80};
  assert(!leitz_native_row(&x,zero,2));
  assert(!leitz_native_row(&x,one,2));
  assert(!leitz_native_row(&x,zero,2));
  assert(!leitz_native_row(&x,one,2));
  assert(!leitz_native_row(&x,zero,2));
  assert(!leitz_native_end(&x));
  rewind(f);
  unsigned w=0,r=0; char error[100]={0};
  assert(!leitz_native_validate(f,&w,&r,error,sizeof error));
  assert(w==16 && r==4); /* Original native Linux omits last blank row */
  rewind(f);
  assert(fseek(f,-1,SEEK_END)==0);
  assert(fputc('X',f)=='X');
  rewind(f);
  assert(leitz_native_validate(f,&w,&r,error,sizeof error)==-1);
  fclose(f);
}
int main(void) {verify(300);verify(600); puts("PASS native protocol validator and malformed-footer rejection");return 0;}
