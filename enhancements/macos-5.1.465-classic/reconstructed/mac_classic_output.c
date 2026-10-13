#define _POSIX_C_SOURCE 200809L
#include "mac_classic_output.h"
#include <limits.h>

ssize_t mac_classic_file_write(void *ctx,const uint8_t *bytes,size_t size) {
  FILE *f=ctx;
  if (!f || (!bytes && size) || size>(size_t)SSIZE_MAX) return -1;
  if (!size) return 0;
  size_t n=fwrite(bytes,1,size,f);
  return n>(size_t)SSIZE_MAX ? -1 : (ssize_t)n;
}

static int write_all(mac_classic_sink_write fn,void *ctx,
                     const uint8_t *bytes,size_t length) {
  if (!fn) return -1;
  size_t sent=0;
  while (sent<length) {
    /* Bound the chunk so a valid ssize_t callback can report it. */
    size_t part=length-sent;
    if (part>(size_t)SSIZE_MAX) part=(size_t)SSIZE_MAX;
    ssize_t n=fn(ctx,bytes+sent,part);
    if (n<=0 || (size_t)n>part) return -1;
    sent+=(size_t)n;
  }
  return 0;
}

int mac_classic_write_data(const mac_classic_output *out,
                           const uint8_t *bytes,size_t length) {
  if (!out || (length && !bytes) || !out->primary ||
      (out->kind!=MAC_CLASSIC_SOCKET && out->kind!=MAC_CLASSIC_STDOUT)) return -1;
  if (!length) return 0;
  if (out->kind==MAC_CLASSIC_STDOUT && out->dump &&
      write_all(out->dump,out->dump_ctx,bytes,length)) return -1;
  return write_all(out->primary,out->primary_ctx,bytes,length);
}
