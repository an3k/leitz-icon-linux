#include "wifi_auth_flow.h"
#include "wifi_auth.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int network_result[2];
    int32_t panel_result;
    unsigned network_calls, panel_calls;
    char trace[4];
    size_t trace_size;
} fixture;

static int observe_network(void *opaque)
{
    fixture *f = opaque;
    assert(f->trace_size < sizeof f->trace);
    f->trace[f->trace_size++] = 'N';
    unsigned call = f->network_calls++;
    assert(call < 2);
    return f->network_result[call];
}

static int32_t observe_panel(void *opaque)
{
    fixture *f = opaque;
    assert(f->trace_size < sizeof f->trace);
    f->trace[f->trace_size++] = 'H';
    ++f->panel_calls;
    assert(f->panel_calls == 1);
    return f->panel_result;
}

static void scenario(int first, int32_t panel_status, int second,
                     int32_t expected, const char *expected_trace)
{
    fixture f = {{first, second}, panel_status, 0, 0, {0}, 0};
    int32_t result = -99;
    assert(leitz_win2018_credentials_flow(observe_network, observe_panel,
                                          &f, &result) == 0);
    assert(result == expected);
    f.trace[f.trace_size] = 0;
    assert(strcmp(f.trace, expected_trace) == 0);
    assert(f.network_calls == (unsigned)(expected == 3 ? 1 : 2));
    assert(f.panel_calls == (unsigned)(first == 1));
}

int main(void)
{
    /* Mismatch skips HTTP entirely, but still checks WLAN again. */
    scenario(0, -99, 0, 4, "NN");
    scenario(0, -99, 1, 2, "NN");

    /* ProtocolError/HTTP 401 is status 1; other exceptions status 2. */
    int32_t unauthorized = leitz_win2018_panel_http_status(
        LEITZ_WIN2018_WEB_EXCEPTION, 7, 401);
    int32_t failed = leitz_win2018_panel_http_status(
        LEITZ_WIN2018_WEB_EXCEPTION, 7, 403);
    assert(unauthorized == 1 && failed == 2);
    scenario(1, unauthorized, 0, 4, "NHN");
    scenario(1, unauthorized, 1, 1, "NHN");
    scenario(1, failed, 0, 4, "NHN");
    scenario(1, failed, 1, 2, "NHN");

    /* Successful GetResponse returns 3 without the second WLAN check. */
    scenario(1, leitz_win2018_panel_http_status(
        LEITZ_WIN2018_HTTP_REPLY, 0, 200), 0, 3, "NH");

    /* New API rejects invalid results without publishing partial state. */
    fixture f = {{1, 1}, 0, 0, 0, {0}, 0};
    int32_t result = 91;
    assert(leitz_win2018_credentials_flow(NULL, observe_panel, &f, &result) == -1);
    assert(leitz_win2018_credentials_flow(observe_network, NULL, &f, &result) == -1);
    assert(leitz_win2018_credentials_flow(observe_network, observe_panel, &f, NULL) == -1);
    assert(f.network_calls == 0 && f.panel_calls == 0 && result == 91);
    f.network_result[0] = -1;
    assert(leitz_win2018_credentials_flow(observe_network, observe_panel,
                                          &f, &result) == -1);
    assert(f.panel_calls == 0 && f.network_calls == 1 && result == 91);
    memset(&f, 0, sizeof f);
    f.network_result[0] = 1;
    f.panel_result = 9;
    assert(leitz_win2018_credentials_flow(observe_network, observe_panel,
                                          &f, &result) == -1);
    assert(f.network_calls == 1 && f.panel_calls == 1 && result == 91);
    memset(&f, 0, sizeof f);
    f.network_result[0] = 1;
    f.network_result[1] = -1;
    f.panel_result = 2;
    assert(leitz_win2018_credentials_flow(observe_network, observe_panel,
                                          &f, &result) == -1);
    assert(f.network_calls == 2 && f.panel_calls == 1 && result == 91);
    puts("Windows 2018 AddPrinter full credential decision ordering: PASS");
    return 0;
}
