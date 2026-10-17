#ifndef LEITZ_WIN2018_GPD_MEDIA_CATALOG_H
#define LEITZ_WIN2018_GPD_MEDIA_CATALOG_H
#include <stddef.h>
#include <stdint.h>

/* Exact manufacturer Windows 2018 GPD PaperSize data.
 * Numeric dimensions are in original MasterUnits PAIR(300,600).
 * Missing fixed dimension/ID = -1. DO NOT infer cassette or cut timing.
 * Separate portable C API, NOT native Unidrv/COM interface.
 */
typedef struct {
    const char *original_id;
    int32_t option_id;
    int32_t resource_name_id;
    int32_t page_dimensions[2];
    int32_t printable_area[2];
    int32_t printable_origin[2];
    int32_t min_size[2];
    int32_t max_size[2];
    uint8_t command[8];
    size_t command_length;
} leitz_win2018_gpd_media_entry;

size_t leitz_win2018_gpd_media_count(void);
const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_at(size_t index);
const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_by_name(const char *original_id);
const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_by_option_id(int32_t option_id);
/* Read-only copy of OEM CmdSelect bytes; 0 on success, -1 on
 * invalid input/capacity. Leaves output and *written unchanged on -1.
 * Does not transmit a print job or interpret the printer's cassette.
 */
int leitz_win2018_gpd_media_copy_command(
    const leitz_win2018_gpd_media_entry *entry,
    uint8_t *output, size_t capacity, size_t *written);
#endif
