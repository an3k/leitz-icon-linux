/* Reconstructed fragments from macOS 5.1.465, classic command2leitz.
 * This is NOT the complete original command2leitz.c. */
#ifndef MAC_CLASSIC_COMMAND_CORE_H
#define MAC_CLASSIC_COMMAND_CORE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  MAC_CLASSIC_UNKNOWN = 0,
  MAC_CLASSIC_READY,
  MAC_CLASSIC_MEDIA_EMPTY,
  MAC_CLASSIC_MEDIA_JAM,
  MAC_CLASSIC_GENERAL_ERROR
} mac_classic_status;

typedef struct {
  const char *command_name;
  uint8_t command;
} mac_classic_query;

/* Verified from GetPrinterStatus/GetUnitedStatus/GetFirmvareVersion at
 * 0x1000011d0, 0x1000012a0, 0x100001370 in command2leitz. */
extern const mac_classic_query mac_classic_queries[3];

/* 0x100001483-0x1000014a8: putchar(ESC); putchar(cmd); fflush(stdout).
 * Return 2 bytes to a caller that owns the actual transport. */
size_t mac_classic_encode_query(uint8_t command, uint8_t output[2]);

/* 0x100001a30-0x100001a75: EndOfJob emits ESC Z independent of flag. */
size_t mac_classic_encode_end_of_job(uint8_t output[2]);

/* 0x1000016b4-0x1000017c4: ESC A response's first byte is compared
 * against 0x03, 0x20, 0x30, 0x40 (not bitfield masking). */
mac_classic_status mac_classic_decode_a(uint8_t value);
const char *mac_classic_status_reason(mac_classic_status status);

#endif
