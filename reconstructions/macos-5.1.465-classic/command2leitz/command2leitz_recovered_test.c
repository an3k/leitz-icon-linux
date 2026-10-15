#include "command2leitz_recovered.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct capture {
  unsigned char stream[32];
  size_t count;
  unsigned flushes;
  char response[1024];
  ssize_t length;
  double timeout;
  char lines[4][2048];
  unsigned messages;
  char log_text[2048];
  unsigned logged;
  unsigned notified;
  unsigned native_end_calls;
  int last_end_flag;
  int log_failure;
};
static int emit_byte(void *p, int c) {
  struct capture *s = p;
  assert(s->count < sizeof(s->stream));
  s->stream[s->count++] = (unsigned char)c;
  return c;
}
static int flush(void *p) { ((struct capture *)p)->flushes++; return 0; }
static ssize_t read_reply(void *p, char *b, int n, double timeout) {
  struct capture *s = p;
  assert(n == 1024);
  s->timeout = timeout;
  if(s->length > 0) memcpy(b, s->response, (size_t)s->length);
  return s->length;
}
static void log_line(void *p, const char *line) {
  struct capture *s=p;
  assert(s->messages < 4 && strlen(line) < sizeof s->lines[0]);
  strcpy(s->lines[s->messages++], line);
}
static int write_log(void *p, const char *line) {
  struct capture *s=p;
  if(s->log_failure)return 0;
  assert(strlen(line)<sizeof s->log_text);
  strcpy(s->log_text,line);
  s->logged++;
  return 1;
}
static void notify(void *p,const char *domain) {
  struct capture *s=p;
  assert(strcmp(domain,"com.xyron.Leitz-Icon-Software")==0);
  s->notified++;
}
static void delegated_end(void *p,int flag) {
  struct capture *s=p;
  ++s->native_end_calls;
  s->last_end_flag=flag;
  emit_byte(p,0x1b);
  emit_byte(p,'Z');
  (void)flush(p);
}
static void reset(struct capture *s) {
  memset(s,0,sizeof *s);
  s->length=1;
}
int main(void) {
  /* Original Mach-O exports four pointer objects in __DATA,__data;
   * these getters must return their actual vendor string payloads. */
  assert(!strcmp(classic_original_printer_status_prefix(),"PRINTER_STATUS_"));
  assert(!strcmp(classic_original_united_status_prefix(),"UNITED_STATUS_"));
  assert(!strcmp(classic_original_firmware_version_prefix(),"FIRMWARE_VERSION_"));
  assert(!strcmp(classic_original_notification_domain(),"com.xyron.Leitz-Icon-Software"));
  struct capture s;
  reset(&s);
  classic_command_io io={&s,emit_byte,flush,read_reply,log_line,write_log,notify,NULL};
  s.response[0]=3;
  assert(classic_command_dispatch(&io,"reportstatus","",0)==1);
  assert(s.count==4 && memcmp(s.stream,"\033A\033Z",4)==0);
  assert(s.flushes==2 && s.timeout==7.0);
  assert(s.messages==2 && strcmp(s.lines[1],"STATE: +com.leitz.ready")==0);
  reset(&s);
  s.length=0;
  assert(classic_command_dispatch(&io,"getunitedstatus","",0)==0);
  assert(s.count==2 && memcmp(s.stream,"\033U",2)==0);
  assert(s.flushes==1 && s.messages==1);
  assert(strcmp(s.lines[0],"ATTR: marker-message=NoData")==0);
  reset(&s);
  s.length=-1;
  assert(classic_command_dispatch(&io,"GetFirmwareVersion","",0)==0);
  assert(s.count==2 && memcmp(s.stream,"\033V",2)==0);
  assert(s.flushes==1 && s.messages==1);
  reset(&s);
  memcpy(s.response,"3.0",3);
  s.length=3;
  assert(classic_command_dispatch(&io,"GetFirmwareVersion","",1)==0);
  assert(s.count==4 && memcmp(s.stream,"\033V\033Z",4)==0);
  assert(strcmp(s.log_text,"FIRMWARE_VERSION_3.0")==0);
  assert(strcmp(s.lines[0],"ATTR: marker-message=\"FIRMWARE_VERSION_3.0\"")==0);
  assert(s.notified==1 && s.logged==1);
  reset(&s);
  s.response[0]='x';s.length=1;s.log_failure=1;
  assert(classic_command_dispatch(&io,"GetUnitedStatus","",1)==0);
  assert(s.count==4 && s.flushes==2 && s.messages==0 && s.notified==0);
  reset(&s);
  assert(classic_command_dispatch(&io,"unknown","",0)==0 && s.count==0);

  /* The original Mach-O exported _GetPrinterStatus, _GetUnitedStatus,
   * _GetFirmvareVersion, _GetStatus and _EndOfJob individually.
   * These are source-level entry-point tests, not native C++ ABI tests.
   */
  reset(&s);
  s.response[0]=0x03;
  assert(GetPrinterStatus(&io,0,7.0)==1);
  assert(s.count==4 && !memcmp(s.stream,"\033A\033Z",4));
  assert(s.messages==2 && !strcmp(s.lines[1],"STATE: +com.leitz.ready"));

  reset(&s);
  s.length=0;
  assert(GetUnitedStatus(&io,0,7.0)==0);
  assert(s.count==2 && !memcmp(s.stream,"\033U",2));
  assert(s.messages==1 && !strcmp(s.lines[0],"ATTR: marker-message=NoData"));

  reset(&s);
  memcpy(s.response,"4.5",3);s.length=3;
  assert(GetFirmvareVersion(&io,1,7.0)==1);
  assert(s.count==4 && !memcmp(s.stream,"\033V\033Z",4));
  assert(!strcmp(s.log_text,"FIRMWARE_VERSION_4.5"));

  reset(&s);
  s.response[0]=0x03;
  assert(GetStatus(&io,'A',"PRINTER_STATUS_",0,7.0)==1);
  assert(s.count==4 && !memcmp(s.stream,"\033A\033Z",4));

  reset(&s);
  EndOfJob(&io,0);
  assert(s.count==2 && !memcmp(s.stream,"\033Z",2));
  assert(s.flushes==1);

  /* Native-only termination bridge on a successful reply: invoke
   * the native boundary ONCE, suppress the callback fallback and
   * preserve byte output, flush count and manufacturer flag. */
  classic_command_io native_route=io;
  native_route.end_job=delegated_end;
  reset(&s);
  s.response[0]=0x03;
  assert(GetStatus(&native_route,'A',"PRINTER_STATUS_",1,7.0)==1);
  assert(s.native_end_calls==1 && s.last_end_flag==1);
  assert(s.count==4 && !memcmp(s.stream,"\033A\033Z",4));
  assert(s.flushes==2);
  /* No successful reply: original _EndOfJob is NOT called. */
  reset(&s);
  s.length=0;
  assert(GetStatus(&native_route,'A',"PRINTER_STATUS_",0,7.0)==0);
  assert(s.native_end_calls==0 && s.count==2 && s.flushes==1);

  puts("command2leitz active-path and native EndOfJob delegation checks passed");
  return 0;
}
