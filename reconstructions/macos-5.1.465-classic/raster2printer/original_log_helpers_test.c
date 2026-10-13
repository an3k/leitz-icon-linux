#include "original_log_helpers.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
  for(int bit=0;bit<32;++bit) {
    int data=(int)(1u<<bit);
    assert(classic_original_is_bit_set(data,bit)==1);
    assert(classic_original_is_bit_set(data,(bit+1)%32)==0);
    assert(classic_original_is_bit_set(data,bit+32)==1);
  }
  classic_original_log_state state={0};
  state.log_file=tmpfile();
  assert(state.log_file!=NULL);
  /* OpenLogFile's already-open branch has no destructive side effects. */
  classic_original_log_open(&state);
  assert(state.log_file!=NULL);
  classic_original_log_close(&state);
  assert(state.log_file==NULL);
  /* Never invoke the global-path open branch in unit tests. */
  puts("Original classic log and bit helper checks passed");
  return 0;
}
