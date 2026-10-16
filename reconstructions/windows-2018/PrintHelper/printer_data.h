#ifndef LEITZ_WIN2018_PRINTER_DATA_H
#define LEITZ_WIN2018_PRINTER_DATA_H
#include <stddef.h>
#include <stdint.h>
/* Windows PrintHelper.dll RetrieveStatus at RVA 0x15f0..0x1715
 * calls Winspool GetPrinterDataW on a named spooler value.
 * This is NOT a live USB/ESC status exchange.
 * New portable callback interface, not the vendor DLL ABI.
 */
#define LEITZ_WIN2018_PRINTER_DATA_LIMIT 128u
typedef int (*leitz_win2018_printer_data_query)(
  void *ctx, const uint16_t *key, uint32_t *type,
  uint8_t *buffer, size_t capacity, size_t *needed);
/* 0: safe retrieval succeeded, -1: failed, no output changes.
 * Reject GetPrinterDataW errors and sizes over OEM stack buffer
 * rather than copying beyond its original fixed 128-byte scratch. */
int leitz_win2018_retrieve_printer_value(
  const uint16_t *key, uint8_t *dst, size_t capacity,
  size_t *length, uint32_t *type,
  leitz_win2018_printer_data_query query, void *context);
#endif
