#ifndef LEITZ_WIN2018_UI_PROPERTY_SHEETS_H
#define LEITZ_WIN2018_UI_PROPERTY_SHEETS_H
#include <stddef.h>
#include <stdint.h>

/* Read-only cross-architecture reconstruction of LeitzUI.dll IPrintOemUI
 * property-sheet *data initialization*. Native callbacks, Win32 memory
 * ownership and pointer fields are deliberately NOT emulated.
 * Manufacturer: x86 functions RVA 0x1500 (document), 0x1630 (printer);
 * x64 RVA 0x14c0 (document), 0x1640 (printer).
 */
typedef enum {
    LEITZ_UI_SHEET_DOCUMENT = 1,
    LEITZ_UI_SHEET_PRINTER = 2
} leitz_win2018_ui_sheet;
typedef enum {
    LEITZ_UI_LAYOUT_X86 = 32,
    LEITZ_UI_LAYOUT_X64 = 64
} leitz_win2018_ui_layout;
typedef enum {
    LEITZ_UI_CALLBACK_DOCUMENT = 1,
    LEITZ_UI_CALLBACK_PRINTER = 2
} leitz_win2018_ui_callback_type;

typedef struct {
    size_t option_item_bytes;
    size_t option_type_bytes;
    size_t option_parameter_bytes;
    size_t selection_offset;
    size_t option_item_zero_byte_offset;
    leitz_win2018_ui_callback_type callback_type;
} leitz_win2018_ui_sheet_info;

/* Without options, the OEM's property sheet method returns required
 * cOptItem=1. With options, OEM zeros exactly cOptItem*item_size,
 * initializes every item cbSize and a specific BYTE=0, then populates
 * only the first item with level=1, flags and initial selection.
 * The selected WORD comes from original object +0x10 (if present).
 * This portable API takes that already-extracted value, NOT any pointer
 * to the native private DEVMODE/OPTITEM layout.
 *
 * A valid buffer/count must contain >=1 item. Real OEM code has unsafe
 * zero-count and allocation failure paths; these are rejected instead.
 * No pointer or callback function address is placed in the output bytes.
 * On validation error, both buffers and *required_count remain unchanged.
 */
int leitz_win2018_ui_property_items(
    leitz_win2018_ui_layout layout,
    leitz_win2018_ui_sheet sheet,
    const uint16_t *initial_selection,
    uint8_t *option_items,
    size_t item_count,
    size_t capacity,
    size_t *required_count,
    leitz_win2018_ui_sheet_info *information);

/* Native OEM makes two zero-initialized allocations for an OPTTYPE and
 * its single OPTPARAM: 0x20/0x18 on x86, 0x30/0x30 on x64. This
 * normalized serialization stores cbSize, one parameter and the
 * option type's original BYTE+2=9, but no native pointer relationships.
 * On error neither output buffer nor *information is changed.
 */
int leitz_win2018_ui_property_type(
    leitz_win2018_ui_layout layout,
    uint8_t *option_type, size_t type_capacity,
    uint8_t *parameter, size_t parameter_capacity);
#endif
