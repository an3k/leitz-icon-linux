#ifndef LEITZ_WINDOWS_2018_WIRELESS_PAYLOAD_H
#define LEITZ_WINDOWS_2018_WIRELESS_PAYLOAD_H
#include <stddef.h>
#include <stdint.h>

/* Portable, OFFLINE model of PrintHelper.dll SetPrinterWirelessNetwork
 * PE32 i386 RVA 0x1150..0x120a. NOT its original Win32 export ABI.
 * The OEM converts the UTF-16 configuration with Windows CP_ACP.
 * An explicit converter callback avoids inventing a global codepage.
 */
typedef int (*leitz_win2018_ansi_convert)(
    void *ctx,const uint16_t *input,size_t units_including_nul,
    uint8_t *destination,size_t capacity,size_t *written_including_nul);
typedef int (*leitz_win2018_wireless_write)(
    void *ctx,const uint8_t *bytes,size_t length);

/* 1 iff conversion and write succeed; 0 if conversion/writer fail,
 * -1 invalid bounds. Caller provides NUL-terminated UTF-16 input
 * within wide_capacity, bounded byte scratch, and the platform's
 * actual ANSI converter. OEM writes strlen(converted) bytes excluding
 * terminating NUL (no additional command prefix in this export).
 * Scratch is securely zeroed after attempted conversion. The callback
 * is only a test/host hook; this module does not access hardware.
 */
int leitz_win2018_prepare_wireless_send(
    const uint16_t *wide_text,size_t wide_capacity,
    uint8_t *scratch,size_t scratch_capacity,
    leitz_win2018_ansi_convert convert,
    leitz_win2018_wireless_write send,void *context);
#endif
