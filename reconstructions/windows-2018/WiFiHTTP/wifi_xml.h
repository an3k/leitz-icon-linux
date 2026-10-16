#ifndef LEITZ_WINDOWS_2018_WIFI_XML_H
#define LEITZ_WINDOWS_2018_WIFI_XML_H
#include <stdint.h>
#include <stddef.h>

/* Portable offline C model derived from original managed .NET IL:
 * AddPrinter GetWiFiScanInfo (MethodDef 267, RVA 0x6548)
 * and GetWiFiNetworkInfo (MethodDef 269, RVA 0x67e4).
 * NOT a .NET ABI or hardware protocol implementation.
 * The root 'response' and first child under 'bss' used in the
 * TEST FIXTURES are inferred from the original selector paths;
 * no actual OEM XML reply has been captured yet.
 */
typedef struct {
    int in_progress;
    uint32_t number_of_results;
    char my_ssid[256];
} leitz_win2018_scan_xml;
typedef struct {
    int valid;
    char name[256];
    int32_t security_mode;
    int32_t network_type;
    int32_t strength;
} leitz_win2018_bss_xml;

/* 0 if decoded, -1 if malformed or beyond explicit bounds.
 * Output is unchanged on error. A scan in progress can omit count
 * and SSID; invalid BSS entries can omit their detail fields.
 * Expat parses XML without network fetching or DTD support.
 */
int leitz_win2018_parse_scan_xml(const char *xml,size_t length,
                                 leitz_win2018_scan_xml *out);
int leitz_win2018_parse_bss_xml(const char *xml,size_t length,
                                leitz_win2018_bss_xml *out);
#endif
