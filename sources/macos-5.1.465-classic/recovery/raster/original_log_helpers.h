#ifndef ORIGINAL_LOG_HELPERS_H
#define ORIGINAL_LOG_HELPERS_H
#include <stdio.h>

/* Verified against original raster2printer x86_64 functions:
 * _OpenLogFile   0x100001910..0x100001b38
 * _CloseLogFile  0x100001e50..0x100001e85
 * _is_bit_set    0x100001630..0x100001656
 * The explicit state is a newly authored representation of original globals.
 */
typedef struct {
  FILE *log_file;
  char filename[1024];
} classic_original_log_state;

/* Side effect: potentially overwrites the OEM global logfile; do not invoke
 * on a real workstation or production printer system. */
void classic_original_log_open(classic_original_log_state *state);
void classic_original_log_close(classic_original_log_state *state);

/* Original x86 bit test for 32-bit integer arguments; shifts use CPU masking. */
int classic_original_is_bit_set(int data,int index);
#endif
