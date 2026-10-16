#ifndef LEITZ_WIN2018_MANAGED_WIFI_H
#define LEITZ_WIN2018_MANAGED_WIFI_H
#include <stddef.h>
#include <stdint.h>

/* Original managed LeitzIconAddPrinterTool.exe (2018) IL, portable API.
 * NOT its .NET ABI and does not contact a printer or network.
 */
const char *leitz_win2018_security_mode(int32_t mode,size_t key_length);

/* Byte-exact OEM form layout only for 7-bit ASCII inputs.
 * Unlike the independent standards-oriented wifi_http.c, the
 * OEM .NET SetWiFiNetwork method replaces '&' with '%26' ONLY
 * in the security key. SSID and mode are NOT similarly escaped.
 * Actual Encoding.Default bytes are Windows codepage-dependent;
 * reject non-ASCII rather than invent native Unicode parity.
 * The output contains a secret; wipe it promptly.
 * Returns 0 success; -1 invalid or insufficient capacity.
 * On failure, output and published length remain unchanged.
 */
int leitz_win2018_oem_ascii_form(const char *ssid,const char *mode,
                                 const char *secret,char *out,
                                 size_t capacity,size_t *written);

typedef enum {
    LEITZ_WIFI_SCAN_DONE=0,
    LEITZ_WIFI_SCAN_POLL_NOW=1,
    LEITZ_WIFI_SCAN_POLL_AFTER_300MS=2,
    LEITZ_WIFI_SCAN_EXHAUSTED=3,
    LEITZ_WIFI_SCAN_INVALID=-1
} leitz_win2018_scan_step;

typedef struct {
    unsigned polls; /* Number of completed follow-up Scan.xml responses */
    int initial_observed;
    int finished;
} leitz_win2018_scan_progress;

/* The caller obtains initial Scan.xml?scan=1, then at most 50
 * follow-up Scan.xml responses. First follow-up is immediate, all
 * subsequent requests follow 300ms waiting. The original also waits
 * 300ms after the final in-progress response before giving up.
 * The caller performs all XML I/O and sleeps, not this model.
 * in_progress must come from exact XML comparison with "1".
 */
leitz_win2018_scan_step leitz_win2018_scan_observe(
    leitz_win2018_scan_progress *state,int in_progress);
#endif
