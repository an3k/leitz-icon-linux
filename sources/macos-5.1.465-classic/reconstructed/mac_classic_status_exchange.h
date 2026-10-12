/* Extracted from command2leitz classic _GetStatus, 0x100001440..0x100001a26.
 * CUPS read and status-line formatting are separate from transport.
 * This is not the whole original command filter.
 */
#ifndef MAC_CLASSIC_STATUS_EXCHANGE_H
#define MAC_CLASSIC_STATUS_EXCHANGE_H
#include "mac_classic_command_core.h"
#include "mac_classic_output.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#define MAC_CLASSIC_STATUS_MAX_BYTES 1024u

typedef ssize_t (*mac_classic_status_read)(void *,uint8_t *,size_t,double);
typedef struct {
  mac_classic_output *output;
  mac_classic_status_read read;
  void *read_ctx;
} mac_classic_status_io;
typedef struct {
  ssize_t reply_length; /* -1: read error, 0: empty, >0: received */
  uint8_t reply[MAC_CLASSIC_STATUS_MAX_BYTES];
  mac_classic_status status_a;  /* only valid when command=='A' and length>0 */
  int end_of_job_sent;
} mac_classic_status_result;

/* Return 1 for data, 0 for empty read, -1 for failure/invalid inputs.
 * The original prints ESC Z only after a positive backchannel response.
 * Unlike the original, this routine guards a 1024-byte response boundary.
 */
int mac_classic_status_query(mac_classic_status_io *io,
                             uint8_t command,double timeout,
                             mac_classic_status_result *result);
/* State messages copied from the original command filter's _GetStatus,
 * 0x1000016da..0x1000017aa. Returns 0 if not an ESC A response. */
const char *mac_classic_state_remove_a(void);
const char *mac_classic_state_add_a(mac_classic_status status);
#endif
