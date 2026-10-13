/* Leitz Icon 5.1.465 classic command filter, functional reconstruction.
 * Separate the observed Mach-O command flow from the CUPS ABI adapter.
 * This is not the original Esselte C source.
 */
#ifndef MAC_CLASSIC_COMMAND_JOB_H
#define MAC_CLASSIC_COMMAND_JOB_H
#include "mac_classic_status_exchange.h"
#include <stdio.h>

typedef int (*mac_classic_emit_line)(void *context, const char *line);
typedef int (*mac_classic_u_v_sink)(void *context, const char *command_name,
                                  const char *value, const uint8_t *data,
                                  size_t length);
typedef struct {
  mac_classic_status_io *status_io;
  mac_classic_emit_line emit;
  void *emit_context;
  mac_classic_u_v_sink archive_u_v; /* optional: macOS file/notify hook */
  void *archive_context;
  double timeout;
} mac_classic_command_context;

/* Returns 1 for processed with nonempty reply, 0 for empty response,
 * -1 on unsupported command, I/O error or no backchannel data.
 * "NoData" and state notifications follow the original GetStatus branch.
 * Unsupported names are not promoted into aliases of original behavior.
 */
int mac_classic_command_run(mac_classic_command_context *context,
                             const char *name, const char *value);

/* Escape binary replies to make CUPS marker-message and diagnostic logs safe.
 * Unlike the original binary's %s formatting, embedded NUL and control bytes
 * are converted to printable escapes. Returns -1 when dst is too small. */
int mac_classic_status_render(char *dst,size_t cap,const uint8_t *data,size_t n);

/* stdout/stderr pipes via CUPS can carry binary bytes; don't buffer requests. */
ssize_t mac_classic_command_write_fd(void *ctx,const uint8_t *data,size_t count);
int mac_classic_command_emit_stderr(void *ctx,const char *line);
#endif
