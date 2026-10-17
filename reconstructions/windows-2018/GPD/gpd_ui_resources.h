#ifndef LEITZ_WIN2018_GPD_UI_RESOURCES_H
#define LEITZ_WIN2018_GPD_UI_RESOURCES_H
#include <stdint.h>
/* Exact original RT_STRING 0x0409 names from x86/x64 LeitzUSRes.dll,
 * 2.17.4.112. Includes literal '*' and original punctuation.
 * Windows GPD CUSTOMSIZE references =USER_DEFINED_SIZE_DISPLAY from
 * StdNames.gpd, not numeric resource 333 in this DLL.
 * Returns NULL for absent or nonnumeric GPD names. */
const char *leitz_win2018_resource_string(uint32_t resource_id);
const char *leitz_win2018_paper_display_name(const char *original_paper_id);
#endif
