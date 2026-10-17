#include "wifi_auth_flow.h"
#include "wifi_auth.h"

int leitz_win2018_credentials_flow(leitz_win2018_network_observer network,
                                  leitz_win2018_panel_observer panel,
                                  void *context, int32_t *status)
{
    if (!network || !panel || !status) return -1;

    /* OEM CheckLogin checks the current WLAN before its Main.html GET.
     * A mismatch returns 2 without making an HTTP request. */
    int first = network(context);
    if (first != 0 && first != 1) return -1;
    int32_t http_status = 2;
    if (first) {
        http_status = panel(context);
        if (http_status < 1 || http_status > 3) return -1;
    }
    int32_t checked = leitz_win2018_check_login_status(first, http_status);

    /* OEM ProcessCredentials returns immediately on successful login;
     * otherwise it observes the WLAN a SECOND time. */
    if (checked == 3) {
        *status = 3;
        return 0;
    }
    int second = network(context);
    if (second != 0 && second != 1) return -1;
    *status = leitz_win2018_process_credentials_status(checked, second);
    return 0;
}
