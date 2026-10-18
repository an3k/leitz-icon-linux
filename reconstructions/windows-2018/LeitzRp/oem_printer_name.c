#include "oem_printer_name.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int leitz_rp_get_printer_name(
    leitz_rp_getprinter_query query, void *context, void *printer_handle,
    uint16_t *destination, size_t capacity_units, size_t *published_units)
{
    if (!query || !printer_handle || !destination || !published_units ||
        capacity_units == 0)
        return -1;

    uint32_t needed = 0;
    const uint16_t *name = NULL;
    size_t name_length = 0;

    /* Native Windows x86 RVA 0x141b and x64 0x1405: level 1,
     * NULL buffer, size 0; ignore this first BOOL return value. */
    (void)query(context, printer_handle, 1u, NULL, 0u, &needed,
                &name, &name_length);
    if (needed == 0u)
        return 0; /* OEM leaves supplied output pointer untouched. */

    uint8_t *temporary = malloc((size_t)needed);
    if (!temporary)
        return -1;

    name = NULL;
    name_length = 0;
    uint32_t second_needed = needed;
    /* Original Windows x86 RVA 0x1449, x64 RVA 0x1443. */
    const int ok = query(context, printer_handle, 1u, temporary,
                         needed, &second_needed, &name, &name_length);
    int result = 0;
    if (ok && name) {
        /* Native uses wcslen and copies a NUL-terminated UTF16 string.
         * A callback's declared bound prevents reading native pointers
         * or unbounded data from a synthetic PRINTER_INFO_1 buffer. */
        if (name_length == SIZE_MAX || name_length >= capacity_units ||
            name_length > (SIZE_MAX / sizeof *destination) - 1u) {
            result = -1;
        } else {
            memcpy(destination, name, name_length * sizeof *destination);
            destination[name_length] = 0;
            *published_units = name_length;
        }
    }
    free(temporary);
    return result;
}
