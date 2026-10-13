#define _POSIX_C_SOURCE 200809L
#include "mac_classic_prn.h"
#include "mac_classic_output.h"
#include <string.h>

int mac_classic_prn_open(mac_classic_prn *p, FILE *sink) {
  if (!p || !sink) return -1;
  memset(p, 0, sizeof *p);
  p->store = tmpfile();
  if (!p->store) return -1;
  p->sink = sink;
  return 0;
}

int mac_classic_prn_write(void *ctx, const uint8_t *bytes, size_t count) {
  mac_classic_prn *p = ctx;
  if (!p || !p->store || (!bytes && count)) return -1;
  if (!count) return 0;
  return fwrite(bytes, 1, count, p->store) == count ? 0 : -1;
}

int mac_classic_prn_replay(mac_classic_prn *p) {
  if (!p || !p->store || !p->sink) return -1;
  if (fflush(p->store) || fseek(p->store, 0, SEEK_SET)) return -1;
  uint8_t buffer[65536];
  mac_classic_output output = {MAC_CLASSIC_STDOUT, mac_classic_file_write,
                              p->sink, NULL, NULL};
  for (;;) {
    size_t got = fread(buffer, 1, sizeof buffer, p->store);
    if (got && mac_classic_write_data(&output, buffer, got)) return -1;
    if (got != sizeof buffer) {
      if (ferror(p->store)) return -1;
      if (feof(p->store)) break;
    }
  }
  return fflush(p->sink) == 0 ? 0 : -1;
}

void mac_classic_prn_close(mac_classic_prn *p) {
  if (!p) return;
  if (p->store) fclose(p->store);
  memset(p, 0, sizeof *p);
}
