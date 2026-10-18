#ifndef LEITZ_WIN2018_RP_OEM_PAPER_SETTING_H
#define LEITZ_WIN2018_RP_OEM_PAPER_SETTING_H
#include <stddef.h>
#include <stdint.h>

/* Original LeitzRp.dll raster vtable slot 12: x86 RVA 0x15f0,
 * x64 RVA 0x1600. Uses IPrintOemDriverUni::DrvGetDriverSetting through
 * host vtable +0x0c (x86) / +0x18 (x64) for ASCII feature "PaperSize".
 * Two calls: first NULL/0 to retrieve cbNeeded, then allocate exactly
 * cbNeeded and retrieve the UTF-16 option name. HRESULTs are ignored by
 * the original. This is a safe portable callback, not native COM ABI. */
typedef int32_t (*leitz_rp_driver_setting_query)(
    void *context, const char *feature, uint8_t *buffer,
    uint32_t capacity_bytes, uint32_t *needed_bytes,
    uint32_t *options_returned);

typedef struct {
    uint32_t first_needed_bytes;
    uint32_t last_needed_bytes;
    uint32_t options_returned;
    uint32_t calls;
    uint32_t has_paper_name;
} leitz_rp_paper_setting_info;

/* Caller supplies maximum allocation as a safety boundary, not as an OEM
 * label-length or stock-format limit. On success writes a terminated UTF-16
 * name to 'name', with length excluding NUL. If cbNeeded is zero, no name
 * is published. The existing label_setup module performs the OEM
 * Continuous/CUSTOMSIZE checks; this module does NOT repeat them.
 * Absent termination, odd cbNeeded, allocation failure or
 * short output capacity safely fail WITHOUT publishing partial outputs.
 * Native second HRESULT is NOT used as a success gate, but the buffer
 * still must contain a bounded, terminated option string. */
int leitz_rp_oem_read_paper_setting(
    leitz_rp_driver_setting_query query, void *context,
    uint32_t max_allocation_bytes,
    uint16_t *name, size_t name_capacity_units, size_t *name_units,
    leitz_rp_paper_setting_info *info);

/* Windows GPD's INTERNAL paper option keys are ASCII; this conversion
 * rejects non-ASCII code units rather than changing their identity.
 * The original UTF-16 option can still be read independently above. */
int leitz_rp_oem_paper_to_ascii(const uint16_t *name, size_t length,
    char *destination, size_t capacity);
#endif
