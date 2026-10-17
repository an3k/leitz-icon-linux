#ifndef LEITZ_WINDOWS_2018_GPD_COMMANDS_H
#define LEITZ_WINDOWS_2018_GPD_COMMANDS_H

#include <stddef.h>
#include <stdint.h>

/* Isolated static Windows 2018 OEM LeitzIcon.gpd command byte recovery.
 * Values refer to the manufacturer's definition, NOT native Unidrv's
 * write callback, ordering, error propagation or an actual USB backend.
 * Fixed-size DHLShippingLabel is the only paper-specific example here;
 * this does not claim any other medium is mapped or supported.
 */
typedef enum {
    LEITZ_WIN2018_GPD_START_DOCUMENT = 0,
    LEITZ_WIN2018_GPD_START_PAGE = 1,
    LEITZ_WIN2018_GPD_END_JOB = 2,
    LEITZ_WIN2018_GPD_DHL_SHIPPING_LABEL = 3
} leitz_win2018_gpd_command;

/* Returns the exact recorded byte count; zero denotes unknown command. */
size_t leitz_win2018_gpd_command_length(leitz_win2018_gpd_command command);

/* Output is changed only on success. Caller's written is unchanged if
 * the command is invalid, capacity is insufficient or output is NULL.
 * Returns 0 for success and -1 otherwise. No I/O or memory allocation.
 */
int leitz_win2018_gpd_copy_command(leitz_win2018_gpd_command command,
                                   uint8_t *output, size_t capacity,
                                   size_t *written);
#endif
