/* Classic macOS raster2printer PrintThroughPRN reconstruction.
 * Origin: _OpenDebugFile 0x100001660, _SendDebugFileToPrinter 0x100001b40,
 * and printThroughPRN selection at 0x10000541d / 0x100006355.
 * The original writes to /Library/Caches/DEBUG/dbg.prn and then replays it
 * to stdout bytewise. This implementation uses an unnamed temporary file
 * to avoid overwriting files and exposes the same deferred output behavior.
 */
#ifndef MAC_CLASSIC_PRN_H
#define MAC_CLASSIC_PRN_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  FILE *store;
  FILE *sink;
} mac_classic_prn;

/* Writes are deferred: nothing goes to sink until replay succeeds. */
int mac_classic_prn_open(mac_classic_prn *p, FILE *sink);
int mac_classic_prn_write(void *ctx, const uint8_t *bytes, size_t count);
int mac_classic_prn_replay(mac_classic_prn *p);
void mac_classic_prn_close(mac_classic_prn *p);
#endif
