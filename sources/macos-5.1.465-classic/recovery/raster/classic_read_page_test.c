#include "classic_read_page.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned header_available;
  unsigned count;
  unsigned size;
  unsigned requested;
} context;

static unsigned get_header(void *p, classic_page_header *h) {
  context *ctx=p;
  ++ctx->count;
  if (!ctx->header_available) return 0;
  h->cups_width=16;
  h->cups_height=2;
  h->cups_bytes_per_line=4;
  h->cups_bits_per_pixel=1;
  h->cups_color_space=0;
  return 1;
}
static unsigned get_pixels(void *p,void *data,unsigned requested) {
  context *ctx=p;
  ctx->requested=requested;
  if (ctx->size) memset(data, 0xa5,ctx->size);
  return ctx->size;
}
int main(void) {
  context ctx={0};
  classic_page_source source={&ctx,get_header,get_pixels};
  classic_page_result result={0};
  unsigned pages=0;
  assert(classic_read_page_core(&source,&pages,&result)==0);
  assert(pages==0 && ctx.count==1);
  ctx.header_available=1;
  ctx.size=8;
  assert(classic_read_page_core(&source,&pages,&result)==1);
  assert(result.page_number==1 && pages==1);
  assert(result.requested_bytes==8 && result.received_bytes==8);
  assert(ctx.requested==8);
  free(result.pixels); result.pixels=NULL;
  /* OEM accepts a nonzero short read; the enhanced project variant did not. */
  ctx.size=3;
  assert(classic_read_page_core(&source,&pages,&result)==1);
  assert(result.requested_bytes==8 && result.received_bytes==3);
  assert(pages==2);
  free(result.pixels); result.pixels=NULL;
  ctx.size=0;
  assert(classic_read_page_core(&source,&pages,&result)==-1);
  assert(pages==3); /* increment happens before zero-read failure */
  free(result.pixels);
  puts("classic ReadPage original branch behavior checks passed");
  return 0;
}
