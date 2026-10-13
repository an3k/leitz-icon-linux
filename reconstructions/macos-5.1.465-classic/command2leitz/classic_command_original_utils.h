#ifndef CLASSIC_COMMAND_ORIGINAL_UTILS_H
#define CLASSIC_COMMAND_ORIGINAL_UTILS_H
#include <stdio.h>

/* Recovered from SHA-256-verified manufacturer command2leitz 5.1.465.
 * The original exported symbols and addresses:
 * _is_bit_set        0x100001c60
 * _FlushDataStreams  0x100001c20
 * _CloseDebugFile    0x100002440
 * _CloseLogFile      0x100002480
 * These routines ignore fflush and fclose errors.
 * The explicit pointer parameters are an offline analysis interface,
 * not the global ABI of the original Mach-O.
 */
int classic_command_original_is_bit_set(unsigned value, unsigned bit);
void classic_command_original_flush(FILE *out, FILE *dump);
void classic_command_original_close_debug(FILE **dump);
void classic_command_original_close_log(FILE **log);
#endif
