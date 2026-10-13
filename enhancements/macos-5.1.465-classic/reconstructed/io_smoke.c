#include "mac_classic_output.h"
#include "mac_classic_prn.h"
#include "mac_classic_status_exchange.h"
#include "mac_classic_command_dispatch.h"
#include <assert.h>
#include <string.h>

typedef struct { uint8_t b[64]; size_t used; size_t chunk; int fail; } sink;
static ssize_t output(void *ctx,const uint8_t *p,size_t n) {
  sink *s=ctx;
  if (s->fail || s->used>=sizeof(s->b)) return -1;
  if (n>s->chunk) n=s->chunk;
  if (n>sizeof(s->b)-s->used) n=sizeof(s->b)-s->used;
  if (!n) return 0;
  memcpy(s->b+s->used,p,n);s->used+=n;return (ssize_t)n;
}
typedef struct { uint8_t buf[32]; ssize_t count; double timeout; } fake_response;
static ssize_t read_reply(void *c,uint8_t *b,size_t n,double timeout) {
  fake_response *r=c;r->timeout=timeout;
  if (r->count<0) return -1;
  if ((size_t)r->count>n) return -1;
  memcpy(b,r->buf,(size_t)r->count);return r->count;
}
int main(void) {
  const uint8_t msg[]={0x41,0x42,0x43};
  sink out={.chunk=1},dump={.chunk=2};
  mac_classic_output router={MAC_CLASSIC_STDOUT,output,&out,output,&dump};
  assert(mac_classic_write_data(&router,msg,sizeof msg)==0);
  assert(out.used==3&&dump.used==3&&memcmp(out.b,msg,3)==0&&memcmp(dump.b,msg,3)==0);
  router.kind=MAC_CLASSIC_SOCKET;
  assert(mac_classic_write_data(&router,msg,sizeof msg)==0 && dump.used==3 && out.used==6);
  out.fail=1;assert(mac_classic_write_data(&router,msg,sizeof msg)<0);out.fail=0;
  assert(mac_classic_write_data(&router,NULL,1)<0);
  router.kind=MAC_CLASSIC_STDOUT;dump.fail=1;
  assert(mac_classic_write_data(&router,msg,sizeof msg)<0 && out.used==6);
  dump.fail=0;

  memset(&out,0,sizeof out);out.chunk=1;
  router=(mac_classic_output){MAC_CLASSIC_STDOUT,output,&out,NULL,NULL};
  fake_response res={.buf={0x30},.count=1};
  mac_classic_status_io io={&router,read_reply,&res};
  mac_classic_status_result result;
  assert(mac_classic_status_query(&io,'A',3.5,&result)==1);
  assert(out.used==4 && memcmp(out.b,"\033A\033Z",4)==0);
  assert(result.status_a==MAC_CLASSIC_MEDIA_JAM && result.end_of_job_sent);
  assert(strcmp(mac_classic_state_add_a(result.status_a),"STATE: +media-jam-error,com.leitz.paper-jam-error")==0);
  assert(strstr(mac_classic_state_remove_a(),"com.leitz.ready")!=NULL);
  memset(&out,0,sizeof out);out.chunk=4;
  res.count=0;
  assert(mac_classic_status_query(&io,'U',1.0,&result)==0);
  assert(out.used==2 && result.reply_length==0 && !result.end_of_job_sent);
  res.count=-1;memset(&out,0,sizeof out);out.chunk=4;
  assert(mac_classic_status_query(&io,'V',1.0,&result)<0);
  assert(out.used==2 && result.reply_length==-1);
  res.count=1;res.buf[0]=0x99;memset(&out,0,sizeof out);out.chunk=4;
  assert(mac_classic_status_query(&io,'A',2.0,&result)==1);
  assert(result.status_a==MAC_CLASSIC_UNKNOWN && !mac_classic_state_add_a(result.status_a));
  /* Original PrintThroughPRN defers the final stdout transfer.
   * Our unnamed temporary file preserves that order without touching
   * /Library/Caches/DEBUG/dbg.prn. */
  FILE *saved=tmpfile();
  assert(saved);
  mac_classic_prn prn={0};
  assert(mac_classic_prn_open(&prn,saved)==0);
  assert(mac_classic_prn_write(&prn,msg,sizeof msg)==0);
  assert(ftell(saved)==0);
  assert(mac_classic_prn_replay(&prn)==0);
  assert(ftell(saved)==3);
  assert(fseek(saved,0,SEEK_SET)==0);
  uint8_t check[3]={0};
  assert(fread(check,1,sizeof check,saved)==sizeof check);
  assert(memcmp(msg,check,sizeof msg)==0);
  mac_classic_prn_close(&prn);
  assert(mac_classic_prn_write(&prn,msg,sizeof msg)<0);
  fclose(saved);

  uint8_t cmd=0;
  assert(mac_classic_command_select("reportSTATUS",&cmd)==1 && cmd=='A');
  assert(mac_classic_command_select("GetUNITEDStatus",&cmd)==1 && cmd=='U');
  assert(mac_classic_command_select("getfirmwareversion",&cmd)==1 && cmd=='V');
  assert(!mac_classic_command_select("GetPrinterStatus",&cmd));
  assert(mac_classic_should_send_esc_a("yes"));
  assert(!mac_classic_should_send_esc_a("NO"));
  return 0;
}
