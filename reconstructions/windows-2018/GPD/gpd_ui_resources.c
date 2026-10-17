#include "gpd_ui_resources.h"
#include "gpd_media_catalog.h"
#include <stddef.h>
static const char *const original_strings[77] = {
#include "gpd_ui_resources_entries.inc"
};
const char *leitz_win2018_resource_string(uint32_t id)
{
    return id >= 259u && id <= 335u ? original_strings[id-259u] : NULL;
}
const char *leitz_win2018_paper_display_name(const char *id)
{
    const leitz_win2018_gpd_media_entry *paper =
        leitz_win2018_gpd_media_by_name(id);
    if (!paper || paper->resource_name_id < 0) return NULL;
    return leitz_win2018_resource_string((uint32_t)paper->resource_name_id);
}
