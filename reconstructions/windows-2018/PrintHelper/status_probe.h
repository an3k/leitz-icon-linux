#ifndef LEITZ_WIN2018_PRINTHELPER_PROBE_H
#define LEITZ_WIN2018_PRINTHELPER_PROBE_H
#include <stddef.h>
#include <stdint.h>

/* Portable source-level reconstruction, NOT the OEM Windows DLL ABI.
 * Original PE32 PrintHelper.dll 2.17.4.112:
 * IsPrinterOnline RVA 0x1510..0x15ef;
 * CanReadFromPrinter RVA 0x1110..0x114a;
 * version helper RVA 0x2270..0x2357.
 */
#define LEITZ_WIN2018_PRINTER_ATTRIBUTE_WORK_OFFLINE UINT32_C(0x00000400)
#define LEITZ_WIN2018_PROBE_VERSION_MAX 64u

/* Reproduces only the boolean decision after OpenPrinterW and two
 * GetPrinterW(level 2) calls. DOES NOT establish a physical USB link.
 * PRINTER_INFO_2 offset +0x34 is Attributes (not Status at +0x48).
 */
int leitz_win2018_is_queue_online(int opened,
                                   int info_retrieved,
                                   uint32_t printer_attributes);

/* Source-derived ESC V probe response classifier. OEM sets boot_marker
 * to 1 if NUL-terminated version text contains uppercase BOOT and
 * does not clear it on absence. CanReadFromPrinter tests the helper's
 * success return (=1), NOT whether the BOOT marker was found.
 * No actual printer I/O. Returns 1 on successful write/read, 0 on
 * failed write/read, -1 for invalid/out-of-bounds offline input.
 */
int leitz_win2018_classify_version_probe(int write_succeeded,
                                          int read_succeeded,
                                          const uint8_t *reply,
                                          size_t received,
                                          int *boot_marker);
#endif
