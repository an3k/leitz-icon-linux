#ifndef LEITZ_WIN2018_MONITOR2_PORT_IO_H
#define LEITZ_WIN2018_MONITOR2_PORT_IO_H
#include <stdint.h>

/* LeitzLm.dll WritePort and ReadPort forwarding, separate from the
 * previously recovered GetPrinterDataFromPort / ESC A,U,V worker.
 * Original monitor table slots 5 and 6, both x86 and x64.
 * Native per-port delegate offsets (x86/x64):
 *    underlying handle +0x0c/+0x18,
 *    WritePort      +0x28/+0x50,
 *    ReadPort       +0x2c/+0x58.
 * The C callbacks below represent this proven forwarding boundary,
 * NOT Windows HANDLEs or native calling conventions. The native code
 * invokes each delegate exactly once and forwards its BOOL result.
 */
typedef int32_t (*leitz_win2018_monitor2_write_callback)(void *context,
    void *underlying_handle, const uint8_t *bytes, uint32_t requested,
    uint32_t *transferred);
typedef int32_t (*leitz_win2018_monitor2_read_callback)(void *context,
    void *underlying_handle, uint8_t *bytes, uint32_t requested,
    uint32_t *transferred);
typedef void (*leitz_win2018_monitor2_last_error)(void *context,
    uint32_t code);
typedef struct {
    void *underlying_handle;
    void *context;
    leitz_win2018_monitor2_write_callback write;
    leitz_win2018_monitor2_read_callback read;
    leitz_win2018_monitor2_last_error error;
} leitz_win2018_monitor2_port;

/* Original WritePort NULL port -> SetLastError(87), return FALSE,
 * and DO NOT touch the output byte count. Native valid-port callback
 * result is returned unchanged, even on partial transfer.
 * Missing native delegates and invalid bounded caller arguments are
 * rejected safely rather than invoking an invalid function pointer. */
int32_t leitz_win2018_monitor2_write(leitz_win2018_monitor2_port *port,
    const uint8_t *bytes, uint32_t requested, uint32_t *transferred);

/* Original ReadPort does not test the monitor port for NULL first;
 * it invokes its read delegate with the original underlying handle
 * and forwards the result. This portable model guards bad inputs,
 * but does not invent SetLastError for this method. */
int32_t leitz_win2018_monitor2_read(leitz_win2018_monitor2_port *port,
    uint8_t *bytes, uint32_t requested, uint32_t *transferred);
#endif
