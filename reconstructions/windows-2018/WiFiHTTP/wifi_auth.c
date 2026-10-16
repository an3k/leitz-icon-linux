#include "wifi_auth.h"

int leitz_win2018_same_network_name(const uint16_t *current,
                                    size_t current_units,
                                    const uint16_t *expected_host,
                                    size_t expected_host_units)
{
    if ((current_units && !current) ||
        (expected_host_units && !expected_host)) return -1;
    if (!current_units || current_units != expected_host_units) return 0;
    for (size_t i=0; i<current_units; ++i)
        if (current[i] != expected_host[i]) return 0;
    return 1;
}

int32_t leitz_win2018_panel_http_status(leitz_win2018_http_outcome outcome,
                                         int32_t webexception_status,
                                         int32_t http_status)
{
    /* Original LeitzIconCommon.dll MethodDef 183 RVA 0x3820:
     * successful GetResponse -> ldc.i4.3;
     * WebException.status==7 and HTTP 401 -> ldc.i4.1;
     * other WebException branches and generic catch -> ldc.i4.2.
     * Main.html GET uses 15000ms timeout, optional NetworkCredential.
     */
    switch (outcome) {
        case LEITZ_WIN2018_HTTP_REPLY: return 3;
        case LEITZ_WIN2018_WEB_EXCEPTION:
            return webexception_status == LEITZ_WIN2018_WEBEXCEPTION_PROTOCOL_ERROR &&
                   http_status == LEITZ_WIN2018_HTTP_UNAUTHORIZED ? 1 : 2;
        case LEITZ_WIN2018_OTHER_EXCEPTION: return 2;
        default: return -1;
    }
}

int32_t leitz_win2018_check_login_status(int first_network_match,
                                         int32_t panel_http_status)
{
    /* Original AddPrinterTool MethodDef 281 RVA 0x6bb8. */
    return first_network_match ? panel_http_status : 2;
}

int32_t leitz_win2018_process_credentials_status(int32_t check_login_status,
                                                 int second_network_match)
{
    /* Original AddPrinterTool MethodDef 279 RVA 0x6b60. */
    if (check_login_status == 3) return 3;
    return second_network_match ? check_login_status : 4;
}
