/* Compile-only native OEM command status function contracts.
 * Original command2leitz Mach-O 5.1.465 provides the argument registers
 * documented in original_command_native_status_entry.h. No real CUPS
 * backchannel, network, or printer call is executed by this compilation. */
#include "original_command_native_status_entry.h"
typedef int (*query_entry)(_Bool,double);
typedef int (*status_entry)(char,_Bool,const char *,double);
static query_entry original_printer=classic_command_native_get_printer_status;
static query_entry original_united=classic_command_native_get_united_status;
static query_entry original_firmware=classic_command_native_get_firmware_version;
static status_entry original_status=classic_command_native_get_status;
int classic_command_native_status_abi_probe(_Bool flag,double timeout,
                                             const char *prefix) {
  return original_printer(flag,timeout)+original_united(flag,timeout)
       +original_firmware(flag,timeout)
       +original_status('U',flag,prefix,timeout);
}
