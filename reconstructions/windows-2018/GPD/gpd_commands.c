#include "gpd_commands.h"
#include <string.h>

#define START_DOCUMENT_ESC_COUNT 121u
static const uint8_t start_page[] = {0x1b, 'B', 0};
static const uint8_t end_job[] = {0x1b, 'E', 0x1b, '@', 0x1b, 'Z'};
static const uint8_t dhl_shipping_label[] = {
    0x1b, 'L', 0x08, 0x36, 0x1b, 'w', 0x03, 0xc0
};

size_t leitz_win2018_gpd_command_length(leitz_win2018_gpd_command command)
{
    switch (command) {
    case LEITZ_WIN2018_GPD_START_DOCUMENT:
        return START_DOCUMENT_ESC_COUNT + 1u;
    case LEITZ_WIN2018_GPD_START_PAGE:
        return sizeof start_page;
    case LEITZ_WIN2018_GPD_END_JOB:
        return sizeof end_job;
    case LEITZ_WIN2018_GPD_DHL_SHIPPING_LABEL:
        return sizeof dhl_shipping_label;
    default:
        return 0u;
    }
}

int leitz_win2018_gpd_copy_command(leitz_win2018_gpd_command command,
                                   uint8_t *output, size_t capacity,
                                   size_t *written)
{
    const size_t n = leitz_win2018_gpd_command_length(command);
    if (!n || !output || !written || capacity < n)
        return -1;
    switch (command) {
    case LEITZ_WIN2018_GPD_START_DOCUMENT:
        /* Observed in the actual OEM GPD: 121 ESC bytes then '@'.
         * Not the older, superseded 120-ESC inventory count. */
        memset(output, 0x1b, START_DOCUMENT_ESC_COUNT);
        output[START_DOCUMENT_ESC_COUNT] = '@';
        break;
    case LEITZ_WIN2018_GPD_START_PAGE:
        memcpy(output, start_page, sizeof start_page);
        break;
    case LEITZ_WIN2018_GPD_END_JOB:
        memcpy(output, end_job, sizeof end_job);
        break;
    case LEITZ_WIN2018_GPD_DHL_SHIPPING_LABEL:
        memcpy(output, dhl_shipping_label, sizeof dhl_shipping_label);
        break;
    default:
        return -1;
    }
    *written = n;
    return 0;
}
