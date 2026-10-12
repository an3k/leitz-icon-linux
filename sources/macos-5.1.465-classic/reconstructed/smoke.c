#include "mac_classic_command_core.h"
#include "mac_classic_raster_boundaries.h"
#include <assert.h>
#include <string.h>
int main(void) {
  uint8_t b[124];
  for (int i = 0; i < 3; i++) {
    assert(mac_classic_encode_query(mac_classic_queries[i].command,b) == 2);
    assert(b[0] == 0x1b && b[1] == mac_classic_queries[i].command);
  }
  assert(mac_classic_encode_end_of_job(b) == 2);
  assert(b[0] == 0x1b && b[1] == 'Z');
  assert(mac_classic_decode_a(0x03) == MAC_CLASSIC_READY);
  assert(mac_classic_decode_a(0x20) == MAC_CLASSIC_MEDIA_EMPTY);
  assert(mac_classic_decode_a(0x30) == MAC_CLASSIC_MEDIA_JAM);
  assert(mac_classic_decode_a(0x40) == MAC_CLASSIC_GENERAL_ERROR);
  assert(mac_classic_decode_a(0x23) == MAC_CLASSIC_UNKNOWN);
  assert(strcmp(mac_classic_status_reason(MAC_CLASSIC_READY),"com.leitz.ready")==0);
  assert(mac_classic_start_document(0,b)==122);
  for (int i=0; i<121; ++i) assert(b[i]==0x1b);
  assert(b[121]=='@');
  assert(mac_classic_start_document(1,b)==124);
  assert(b[122]==0x1b && b[123]=='i');
  assert(mac_classic_end_page(0,b)==2);
  assert(memcmp(b,"\033G",2)==0);
  assert(mac_classic_end_page(1,b)==4);
  assert(memcmp(b,"\033G\033E",4)==0);
  assert(mac_classic_end_document(0,0,b)==6);
  assert(memcmp(b,"\033E\033@\033Z",6)==0);
  assert(mac_classic_end_document(0,1,b)==4);
  assert(memcmp(b,"\033@\033Z",4)==0);
  assert(mac_classic_end_document(1,0,b)==4);
  assert(memcmp(b,"\033@\033Z",4)==0);
  return 0;
}
