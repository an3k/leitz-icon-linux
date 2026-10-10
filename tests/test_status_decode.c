#include "../src/leitz_status.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void check_code(leitz_reply_kind k,uint8_t b,const char *expect,int ambiguous) {
  leitz_reply x;
  assert(leitz_parse_reply(k,&b,1,&x)==0);
  assert(!strcmp(x.interpretation,expect));
  assert(x.status_byte==b && x.ambiguous==ambiguous);
}
int main(void) {
  leitz_reply x;
  check_code(LEITZ_REPLY_STATUS_LEGACY,0x03,"READY_LEGACY",0);
  check_code(LEITZ_REPLY_STATUS_LEGACY,0x20,"MEDIA_EMPTY_LEGACY",0);
  check_code(LEITZ_REPLY_STATUS_LEGACY,0x30,"PAPER_JAM_LEGACY",0);
  check_code(LEITZ_REPLY_STATUS_LEGACY,0x40,"GENERAL_ERROR_LEGACY",0);
  check_code(LEITZ_REPLY_STATUS_LEGACY,0x00,"UNKNOWN_LEGACY",0);
  check_code(LEITZ_REPLY_STATUS_V3,0x03,"READY_V3_UNVERIFIED",0);
  check_code(LEITZ_REPLY_STATUS_V3,0x04,"DOOR_OPEN_V3_UNVERIFIED",0);
  check_code(LEITZ_REPLY_STATUS_V3,0xc0,"FAULT_OR_BUSY_V3_AMBIGUOUS",1);
  check_code(LEITZ_REPLY_STATUS_V3,0xa0,"FAULT_OR_BUSY_V3_AMBIGUOUS",1);
  check_code(LEITZ_REPLY_STATUS_V3,0x90,"FAULT_OR_BUSY_V3_AMBIGUOUS",1);
  check_code(LEITZ_REPLY_STATUS_V3,0x88,"FAULT_OR_BUSY_V3_AMBIGUOUS",1);
  check_code(LEITZ_REPLY_STATUS_V3,0x00,"BUSY_V3_UNVERIFIED",0);
  check_code(LEITZ_REPLY_STATUS_V3,0xff,"UNKNOWN_V3",0);
  const uint8_t p1[]={0,0,1,0x2c},p2[]={0},p3[]={0xff,0xff,0xff,0xff};
  assert(!leitz_parse_reply(LEITZ_REPLY_PRINTED_LINES,p1,sizeof p1,&x));
  assert(x.printed_lines==300);
  assert(!leitz_parse_reply(LEITZ_REPLY_PRINTED_LINES,p2,sizeof p2,&x));
  assert(x.printed_lines==0);
  assert(!leitz_parse_reply(LEITZ_REPLY_PRINTED_LINES,p3,sizeof p3,&x));
  assert(x.printed_lines==UINT32_MAX);
  const uint8_t badlength[]={0,0,0,0,0};
  assert(leitz_parse_reply(LEITZ_REPLY_PRINTED_LINES,badlength,sizeof badlength,&x)==-1);
  const uint8_t v1[]={'1','.','2','3',0},v2[]={'v','2'},v3[]={'1',0,'X'},v4[]={0};
  assert(!leitz_parse_reply(LEITZ_REPLY_VERSION,v1,sizeof v1,&x));
  assert(!strcmp(x.version,"1.23"));
  assert(!leitz_parse_reply(LEITZ_REPLY_VERSION,v2,sizeof v2,&x));
  assert(!strcmp(x.version,"v2"));
  assert(leitz_parse_reply(LEITZ_REPLY_VERSION,v3,sizeof v3,&x)==-1);
  assert(leitz_parse_reply(LEITZ_REPLY_VERSION,v4,sizeof v4,&x)==-1);
  assert(leitz_parse_reply(LEITZ_REPLY_STATUS_LEGACY,p1,0,&x)==-1);
  assert(leitz_parse_reply(LEITZ_REPLY_STATUS_LEGACY,p1,1025,&x)==-1);
  assert(leitz_parse_reply(LEITZ_REPLY_STATUS_LEGACY,p1,1,NULL)==-1);
  assert(leitz_parse_reply(LEITZ_REPLY_STATUS_LEGACY,NULL,1,&x)==-1);
  leitz_reply_kind k;
  assert(!leitz_reply_kind_from_name("legacy",&k) && k==LEITZ_REPLY_STATUS_LEGACY);
  assert(!leitz_reply_kind_from_name("v3",&k) && k==LEITZ_REPLY_STATUS_V3);
  assert(!leitz_reply_kind_from_name("lines",&k) && k==LEITZ_REPLY_PRINTED_LINES);
  assert(!leitz_reply_kind_from_name("version",&k) && k==LEITZ_REPLY_VERSION);
  assert(leitz_reply_kind_from_name("nope",&k)==-1);
  puts("PASS status response decoders: legacy/V3 binary byte, printed lines, version, malformed replies");
  return 0;
}
