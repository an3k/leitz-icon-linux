#include "gpd_media_catalog.h"
#include <string.h>

static const leitz_win2018_gpd_media_entry entries[] = {
#include "gpd_media_catalog_entries.inc"
};

size_t leitz_win2018_gpd_media_count(void)
{
    return sizeof entries / sizeof entries[0];
}

const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_at(size_t index)
{
    return index < leitz_win2018_gpd_media_count() ? &entries[index] : NULL;
}

const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_by_name(const char *id)
{
    if (!id) return NULL;
    for (size_t i = 0; i < leitz_win2018_gpd_media_count(); ++i)
        if (strcmp(entries[i].original_id, id) == 0) return &entries[i];
    return NULL;
}

const leitz_win2018_gpd_media_entry *leitz_win2018_gpd_media_by_option_id(int32_t id)
{
    if (id < 0) return NULL; /* CUSTOMSIZE has no OEM OptionID. */
    for (size_t i = 0; i < leitz_win2018_gpd_media_count(); ++i)
        if (entries[i].option_id == id) return &entries[i];
    return NULL;
}

int leitz_win2018_gpd_media_copy_command(
    const leitz_win2018_gpd_media_entry *entry,
    uint8_t *output, size_t capacity, size_t *written)
{
    if (!entry || !output || !written || !entry->command_length ||
        entry->command_length > sizeof entry->command ||
        entry->command_length > capacity) return -1;
    memcpy(output, entry->command, entry->command_length);
    *written = entry->command_length;
    return 0;
}
