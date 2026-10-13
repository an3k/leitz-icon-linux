/* Disassembly-derived C fragments; not a complete source reconstruction.
 * Original: LeitzIconSoftware-Installer_5.1.465.dmg.zip,
 * classic Mach-O executable command2leitz, x86-64.
 * Each reconstructed function is documented with its Mach-O address.
 */
#include "mac_classic_command_core.h"

const mac_classic_query mac_classic_queries[3] = {
  {"GetPrinterStatus", 'A'},
  {"GetUnitedStatus", 'U'},
  {"GetFirmvareVersion", 'V'} /* spelling preserved from original symbol */
};

size_t mac_classic_encode_query(uint8_t command, uint8_t output[2]) {
  if (!output) return 0;
  output[0] = 0x1b;
  output[1] = command;
  return 2;
}

size_t mac_classic_encode_end_of_job(uint8_t output[2]) {
  return mac_classic_encode_query((uint8_t)'Z', output);
}

mac_classic_status mac_classic_decode_a(uint8_t value) {
  /* GetStatus's single-byte equality checks: no invented bitmask. */
  switch (value) {
    case 0x03: return MAC_CLASSIC_READY;
    case 0x20: return MAC_CLASSIC_MEDIA_EMPTY;
    case 0x30: return MAC_CLASSIC_MEDIA_JAM;
    case 0x40: return MAC_CLASSIC_GENERAL_ERROR;
    default:   return MAC_CLASSIC_UNKNOWN;
  }
}

const char *mac_classic_status_reason(mac_classic_status status) {
  /* Mirrors messages in classic command2leitz's GetStatus branch. */
  switch (status) {
    case MAC_CLASSIC_READY:         return "com.leitz.ready";
    case MAC_CLASSIC_MEDIA_EMPTY:   return "com.leitz.out-of-paper-warning,media-empty-warning";
    case MAC_CLASSIC_MEDIA_JAM:     return "media-jam-error,com.leitz.paper-jam-error";
    case MAC_CLASSIC_GENERAL_ERROR: return "com.leitz.general-error";
    default:                        return "";
  }
}
