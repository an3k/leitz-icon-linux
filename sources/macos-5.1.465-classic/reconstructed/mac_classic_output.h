/* Classic macOS Leitz Icon 5.1.465: bounded reconstruction of WriteData.
 * Executables: raster2printer _WriteData at 0x100001450 and
 * command2leitz _WriteData at 0x100001a80.
 * Their original three paths are socket send, stdout fwrite and optional
 * PRN dump before stdout. The explicit selection and full-write validation
 * are intentional hardening, NOT assertions about the original source.
 */
#ifndef MAC_CLASSIC_OUTPUT_H
#define MAC_CLASSIC_OUTPUT_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef ssize_t (*mac_classic_sink_write)(void *, const uint8_t *, size_t);
typedef enum { MAC_CLASSIC_STDOUT = 0, MAC_CLASSIC_SOCKET = 1 } mac_classic_output_kind;
typedef struct {
  mac_classic_output_kind kind;
  mac_classic_sink_write primary;
  void *primary_ctx;
  mac_classic_sink_write dump;
  void *dump_ctx;
} mac_classic_output;

/* Returns 0 only after ALL requested bytes have reached the selected sink.
 * A dump is attempted before the primary stream for STDOUT mode only.
 * Dump errors are fatal by design; the original ignored them.
 */
int mac_classic_write_data(const mac_classic_output *out,
                           const uint8_t *bytes, size_t length);
ssize_t mac_classic_file_write(void *ctx, const uint8_t *bytes, size_t size);
#endif
