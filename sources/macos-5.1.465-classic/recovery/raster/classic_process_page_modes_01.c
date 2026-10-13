#include "classic_process_page_modes_01.h"
#include <string.h>

/* _ProcessPage starts modes 0 and 1 with ESC D <width> SYN exactly once.
 * Mode 0 transfers all row bytes, including completely blank rows.
 * Mode 1 skips all-zero rows by delaying a 16-bit blank-line counter,
 * emitting ESC f <hi> <lo> only when a later nonblank row arrives.
 * The 16-bit counter's extreme overflow behavior still needs a direct
 * original-executable comparison, so no claim is made about that case.
 */
void classic_mode01_begin(classic_mode01_state *state,unsigned mode,
                          unsigned packed_stride,uint8_t output[4]) {
  state->optimization=mode;
  state->packed_stride=packed_stride;
  state->pending_empty_rows=0;
  output[0]=0x1b;
  output[1]='D';
  output[2]=(uint8_t)packed_stride;
  output[3]=0x16;
}

size_t classic_mode01_row(classic_mode01_state *state,
                          const uint8_t *row,uint8_t *output) {
  size_t n=0;
  unsigned x;
  if (state->optimization==1) {
    for (x=0; x<state->packed_stride; ++x) {
      if (row[x]!=0) break;
    }
    if (x==state->packed_stride) {
      ++state->pending_empty_rows;
      return 0;
    }
    if (state->pending_empty_rows) {
      output[n++]=0x1b;
      output[n++]='f';
      output[n++]=(uint8_t)(state->pending_empty_rows>>8);
      output[n++]=(uint8_t)state->pending_empty_rows;
      state->pending_empty_rows=0;
    }
  }
  memcpy(output+n,row,state->packed_stride);
  return n+state->packed_stride;
}
