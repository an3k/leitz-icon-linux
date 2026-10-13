#include "original_status_gate.h"
#include <string.h>
/* 0x9191c8: if _terminate_flag nonzero, branch to 0x9208.
 * Otherwise test "ipp" at descriptor +0x50: if not present or
 * _sock!=-1, branch to 0x9243 (status path).
 * 0x9208..0x923e: check "ipp" again and invoke _ResolveDeviceURI,
 * then immediately return via 0x99f2.
 * This helper captures only that entry choice, not the socket I/O.
 */
int classic_original_status_uses_resolver(const char *uri,
                                           int socket_fd,
                                           int terminate_flag) {
  if(!terminate_flag && (!strstr(uri,"ipp") || socket_fd!=-1))
    return 0;
  if(strstr(uri,"ipp")) return 1;
  return 0;
}
