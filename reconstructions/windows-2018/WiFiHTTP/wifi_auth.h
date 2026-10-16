#ifndef LEITZ_WIN2018_WIFI_AUTH_H
#define LEITZ_WIN2018_WIFI_AUTH_H
#include <stddef.h>
#include <stdint.h>

/* Original Windows 2018 AddPrinterTool managed methods:
 * 460 IsConnectedToNetwork RVA 0x8334;
 * 281 CheckLogin RVA 0x6bb8;
 * 279 ProcessCredentials RVA 0x6b60;
 * 264 IsPrinterControlPanelAvailable RVA 0x62d4.
 * Original LeitzIconCommon.dll MethodDef 183:
 * WebSiteIsAvailable RVA 0x3820.
 * Portable OFFLINE decision model only, NOT the OEM .NET ABI or
 * any authenticated network/device transport.
 */
#define LEITZ_WIN2018_PANEL_HTTP_TIMEOUT_MS 15000u
#define LEITZ_WIN2018_WIFI_SCAN_HTTP_TIMEOUT_MS 18000u
#define LEITZ_WIN2018_WEBEXCEPTION_PROTOCOL_ERROR 7
#define LEITZ_WIN2018_HTTP_UNAUTHORIZED 401

typedef enum {
    LEITZ_WIN2018_HTTP_REPLY = 0,
    LEITZ_WIN2018_WEB_EXCEPTION = 1,
    LEITZ_WIN2018_OTHER_EXCEPTION = 2
} leitz_win2018_http_outcome;

/* .NET IsNullOrEmpty(currentSSID), then currentSSID.Equals(host)
 * as a case-sensitive UTF-16 code-unit string comparison. No DNS or
 * case normalization. Returns -1 on a missing nonempty span, else 0/1.
 */
int leitz_win2018_same_network_name(const uint16_t *current,
                                    size_t current_units,
                                    const uint16_t *expected_host,
                                    size_t expected_host_units);

/* WebSiteIsAvailable: successful GetResponse => 3; WebException
 * ProtocolError (7) + HTTP 401 => 1; other exceptions => 2.
 * Invalid outcomes return -1 (new bounded-interface validation).
 */
int32_t leitz_win2018_panel_http_status(leitz_win2018_http_outcome outcome,
                                         int32_t webexception_status,
                                         int32_t http_status);

/* CheckLogin: failed first network match => 2 without probing HTTP,
 * otherwise the actual control-panel HTTP status.
 */
int32_t leitz_win2018_check_login_status(int first_network_match,
                                         int32_t panel_http_status);

/* ProcessCredentials: status 3 short-circuits. Otherwise it performs
 * a SECOND network check and overrides status with 4 on mismatch.
 */
int32_t leitz_win2018_process_credentials_status(int32_t check_login_status,
                                                 int second_network_match);
#endif
