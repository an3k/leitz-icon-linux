#include "classic_process_page_modes_01.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
  classic_mode01_state state;
  uint8_t header[4], output[12];
  const uint8_t empty[3]={0,0,0};
  const uint8_t ink[3]={0,1,0};
  classic_mode01_begin(&state,0,3,header);
  assert(memcmp(header,"\033D\003\026",4)==0);
  assert(classic_mode01_row(&state,empty,output)==3);
  assert(memcmp(output,empty,3)==0);
  assert(classic_mode01_row(&state,ink,output)==3);
  assert(memcmp(output,ink,3)==0);
  classic_mode01_begin(&state,1,3,header);
  assert(memcmp(header,"\033D\003\026",4)==0);
  assert(classic_mode01_row(&state,empty,output)==0);
  assert(state.pending_empty_rows==1);
  assert(classic_mode01_row(&state,empty,output)==0);
  assert(state.pending_empty_rows==2);
  assert(classic_mode01_row(&state,ink,output)==7);
  const uint8_t expected[7]={0x1b,'f',0,2,0,1,0};
  assert(memcmp(output,expected,7)==0);
  assert(state.pending_empty_rows==0);
  assert(classic_mode01_row(&state,ink,output)==3);
  assert(memcmp(output,ink,3)==0);
  assert(classic_mode01_row(&state,empty,output)==0);
  assert(state.pending_empty_rows==1); /* trailing blank stays deferred */
  puts("classic ProcessPage modes 0/1 framing checks passed");
  return 0;
}
