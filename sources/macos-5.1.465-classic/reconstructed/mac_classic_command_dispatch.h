/* command2leitz 5.1.465 classic: recovered case-insensitive command selection.
 * _main 0x1000010b4..0x100001176, using strcasecmp() after cupsFileGetConf().
 */
#ifndef MAC_CLASSIC_COMMAND_DISPATCH_H
#define MAC_CLASSIC_COMMAND_DISPATCH_H
#include <stdint.h>
/* Original CUPS command option -> command letter for _GetStatus().
 * Returns 1 when a command is recognized; 0 otherwise.
 * Input must be the name extracted by cupsFileGetConf, not a whole line.
 */
int mac_classic_command_select(const char *name, uint8_t *command);
/* The original command filter compares this PPD choice to "YES" using
 * strcasecmp(), forwarding the result to _EndOfJob. That original routine
 * unconditionally emits ESC Z regardless of the boolean. */
int mac_classic_should_send_esc_a(const char *choice);
#endif
