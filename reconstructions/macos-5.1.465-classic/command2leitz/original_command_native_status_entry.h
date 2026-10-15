#ifndef ORIGINAL_COMMAND_NATIVE_STATUS_ENTRY_H
#define ORIGINAL_COMMAND_NATIVE_STATUS_ENTRY_H
/* Authenticated original macOS 5.1.465 Mach-O command2leitz:
 * _GetPrinterStatus 0x1000011d8 / _GetUnitedStatus 0x1000012a8 /
 * _GetFirmvareVersion 0x100001378:
 *   bool should_send_esc_a in %dil (masked to one bit)
 *   double timeout in %xmm0.
 * Each forwards to _GetStatus at 0x100001287, 0x100001357,
 * 0x100001427 respectively, with:
 *   command in %dil, bool flag in %sil, const char *prefix in %rdx,
 *   double timeout in %xmm0.
 * This recovers the native function signature, not every private
 * C++ global or instruction-level aspect of the OEM status routine.
 * _GetStatus 0x1000019e0..0x1000019eb forwards the masked bool flag
 * in %edi to _EndOfJob, which has one bool parameter. The existing
 * recovered ESC Z / stdout flush body is reused without duplication.
 * The existing single recovered status body and CUPS callbacks are
 * reused; no alternate status algorithm is introduced.
 */
#if defined(__APPLE__)
int classic_command_native_get_status(char command,_Bool should_send_esc_a,
                                       const char *prefix,double timeout)
    __asm__("_GetStatus");
int classic_command_native_get_printer_status(_Bool should_send_esc_a,
                                               double timeout)
    __asm__("_GetPrinterStatus");
int classic_command_native_get_united_status(_Bool should_send_esc_a,
                                              double timeout)
    __asm__("_GetUnitedStatus");
int classic_command_native_get_firmware_version(_Bool should_send_esc_a,
                                                 double timeout)
    __asm__("_GetFirmvareVersion");
void classic_command_native_end_of_job(_Bool should_send_esc_a)
    __asm__("_EndOfJob");
#else
int classic_command_native_get_status(char,_Bool,const char *,double);
int classic_command_native_get_printer_status(_Bool,double);
int classic_command_native_get_united_status(_Bool,double);
int classic_command_native_get_firmware_version(_Bool,double);
void classic_command_native_end_of_job(_Bool);
#endif
#endif
