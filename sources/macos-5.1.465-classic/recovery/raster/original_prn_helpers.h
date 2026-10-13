#ifndef ORIGINAL_PRN_HELPERS_H
#define ORIGINAL_PRN_HELPERS_H
#include <stdio.h>

/* Original classic raster2printer 5.1.465:
 * _OpenDebugFile          0x100001660..0x100001900
 * _SendDebugFileToPrinter 0x100001b40..0x100001e0c
 * The state is an analysis-level representation of two original globals,
 * not the original ABI or a complete printing driver.
 */
typedef struct {
  FILE *dump;
  char filename[1024];
} classic_original_prn_state;

/* OEM path and permissions; calling this can overwrite a global PRN file.
 * Do not call in production or on an ordinary workstation.
 */
void classic_original_prn_open(classic_original_prn_state *state);
void classic_original_prn_close(classic_original_prn_state *state);

/* The new output FILE parameter is for isolated comparison. The actual OEM
 * calls fputc(..., stdout). All branching and diagnostics below reflect
 * the observed manufacturer code, including unchecked I/O operations.
 */
int classic_original_prn_send(classic_original_prn_state *state,FILE *output);
#endif
