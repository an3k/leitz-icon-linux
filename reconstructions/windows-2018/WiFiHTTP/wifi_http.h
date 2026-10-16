#ifndef LEITZ_WIN2018_WIFI_HTTP_H
#define LEITZ_WIN2018_WIFI_HTTP_H
#include <stddef.h>
#include <stdint.h>

/* OEM 2018 managed AddPrinterTool: GetWiFiScanInfo RVA 0x6548,
 * GetWiFiNetworkInfo RVA 0x67e4, SetWiFiNetwork RVA 0x6960.
 * Portable OFFLINE reconstruction, NOT original .NET/HTTP ABI.
 */
int leitz_win2018_wifi_scan_path(char *out,size_t cap,size_t *length);
int leitz_win2018_wifi_scan_row_path(uint32_t row,char *out,
                                     size_t cap,size_t *length);

/* Reconstructs the exact OEM form FIELD NAMES AND ORDER while
 * percent-encoding caller-provided UTF-8 bytes according to RFC 3986.
 * The application demonstrably replaces '&' by '%26', but its entire
 * Unicode/codepage escaping is not proven. Therefore byte-for-byte
 * OEM parity is asserted ONLY for unreserved ASCII values.
 * The output contains a network secret. Caller must zeroize it.
 * No network or device operations take place.
 * Returns 0 on success, -1 on invalid input/capacity; on failure
 * output and *length remain untouched.
 */
int leitz_win2018_wifi_infrastructure_form(
    const uint8_t *ssid,size_t ssid_len,
    const uint8_t *mode,size_t mode_len,
    const uint8_t *secret,size_t secret_len,
    char *out,size_t capacity,size_t *length);

void leitz_win2018_wifi_clear(void *buffer,size_t size);
#endif
