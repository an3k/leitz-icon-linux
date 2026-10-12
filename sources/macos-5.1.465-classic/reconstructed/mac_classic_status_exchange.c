#include "mac_classic_status_exchange.h"
#include <string.h>

const char *mac_classic_state_remove_a(void) {
  return "STATE: -com.leitz.out-of-paper-warning,media-empty-warning,media-jam-error,com.leitz.ready,com.leitz.general-error,com.leitz.paper-jam-error";
}
const char *mac_classic_state_add_a(mac_classic_status s) {
  switch (s) {
    case MAC_CLASSIC_READY:return "STATE: +com.leitz.ready";
    case MAC_CLASSIC_MEDIA_EMPTY:return "STATE: +com.leitz.out-of-paper-warning,media-empty-warning";
    case MAC_CLASSIC_MEDIA_JAM:return "STATE: +media-jam-error,com.leitz.paper-jam-error";
    case MAC_CLASSIC_GENERAL_ERROR:return "STATE: +com.leitz.general-error";
    default:return NULL;
  }
}

int mac_classic_status_query(mac_classic_status_io *io,
                             uint8_t command,double timeout,
                             mac_classic_status_result *result) {
  if (!io || !io->output || !io->read || !result ||
      (command!='A' && command!='U' && command!='V') ||
      !(timeout>=0.0) || timeout>86400.0) return -1;
  memset(result,0,sizeof(*result));
  result->reply_length=-1;
  uint8_t request[2]={0x1b,command};
  if (mac_classic_write_data(io->output,request,sizeof request)) return -1;
  ssize_t read=io->read(io->read_ctx,result->reply,
                         sizeof result->reply,timeout);
  if (read<0 || (size_t)read>sizeof result->reply) return -1;
  result->reply_length=read;
  if (!read) return 0;
  if (command=='A') result->status_a=mac_classic_decode_a(result->reply[0]);
  /* _GetStatus() -> _EndOfJob(): 0x1000019e0; 0x100001a30. */
  uint8_t end[]={0x1b,'Z'};
  if (mac_classic_write_data(io->output,end,sizeof end)) return -1;
  result->end_of_job_sent=1;
  return 1;
}
