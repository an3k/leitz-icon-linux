#ifndef LEITZ_WIN2018_WIFI_AUTH_FLOW_H
#define LEITZ_WIN2018_WIFI_AUTH_FLOW_H
#include <stdint.h>

/* Offline composition of Windows 2018 AddPrinterTool methods:
 * ProcessCredentials (MethodDef 279, RVA 0x6b60) and
 * CheckLogin (MethodDef 281, RVA 0x6bb8).
 * Callbacks are synthetic observations, not HTTP/USB operations.
 * network() reports 0 (mismatch), 1 (match), or an error.
 * panel() returns the reconstructed 1/2/3 WebSiteIsAvailable status.
 */
typedef int (*leitz_win2018_network_observer)(void *context);
typedef int32_t (*leitz_win2018_panel_observer)(void *context);

/* On success writes result status 1, 2, 3, or 4 and returns 0.
 * On invalid pointers, callback errors or unsupported statuses,
 * returns -1 without changing *status. Deliberate safe extension.
 */
int leitz_win2018_credentials_flow(leitz_win2018_network_observer network,
                                  leitz_win2018_panel_observer panel,
                                  void *context, int32_t *status);
#endif
