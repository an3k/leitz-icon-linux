#ifndef LEITZ_WIN2018_RP_OEM_PRINTER_NAME_H
#define LEITZ_WIN2018_RP_OEM_PRINTER_NAME_H
#include <stddef.h>
#include <stdint.h>

/* Original manufacturer LeitzRp.dll vtable method 7:
 * x86 RVA 0x1370, x64 RVA 0x1350, GetPrinterW(PRINTER_INFO_1, level=1).
 * Portable C model, NOT the Windows COM, PRINTER_INFO_1 or GDI ABI.
 * The caller owns the output and provides the printer handle.
 */
typedef int (*leitz_rp_getprinter_query)(
    void *context, void *printer_handle, uint32_t level,
    uint8_t *temporary, uint32_t supplied_bytes, uint32_t *required_bytes,
    const uint16_t **name_utf16, size_t *name_units);

/* Read native PRINTER_INFO_1W pName using the original two-pass
 * GetPrinterW query. First call always has NULL/0, and its BOOL result
 * is ignored: its pcbNeeded alone controls the second call. The second
 * calls GetPrinterW(level=1, buffer, required_bytes, ...). Only on
 * a nonzero BOOL result is the returned pName copied.
 *
 * Valid requests return 0, even if no printer name was found (matching
 * OEM's always-zero HRESULT). For missing/invalid caller output or
 * insufficient safe buffer capacity, return -1 without publishing data.
 * Output length excludes terminal UTF16 NUL; no output is altered when
 * no name is obtained. Temporary OS buffer is always freed safely;
 * original alloc/free ownership is not emulated.
 *
 * Original PRINTER_INFO_1W.pName resides at +0x08 (x86), +0x10 (x64),
 * but real native pointer decoding is deliberately externalized in
 * query callback. It must return a borrowed, bounded UTF16 string.
 */
int leitz_rp_get_printer_name(
    leitz_rp_getprinter_query query, void *context, void *printer_handle,
    uint16_t *destination, size_t capacity_units, size_t *published_units);
#endif
