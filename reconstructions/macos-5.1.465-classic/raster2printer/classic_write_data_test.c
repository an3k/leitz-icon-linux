#include "classic_write_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  char events[8];
  unsigned events_len;
  size_t socket_return;
  size_t stdout_return;
  size_t dump_return;
  size_t requested;
} capture;
static ssize_t send_one(void *p,const void *bytes,size_t len) {
  capture *c=p; (void)bytes; c->events[c->events_len++]='S';
  c->requested=len;return (ssize_t)c->socket_return;
}
static size_t write_one(void *p,const void *bytes,size_t len) {
  capture *c=p; (void)bytes; c->events[c->events_len++]='O';
  c->requested=len;return c->stdout_return;
}
static size_t dump_one(void *p,const void *bytes,size_t len) {
  capture *c=p; (void)bytes; c->events[c->events_len++]='D';
  c->requested=len;return c->dump_return;
}
static void clear(capture *c) { memset(c,0,sizeof(*c)); }
int main(void) {
  capture c={0};
  classic_write_data_io io={&c,1,1,1,send_one,write_one,dump_one};
  const char text[]="raster";
  c.socket_return=1;
  assert(classic_write_data_core(&io,text,6)==1);
  assert(c.requested==6 && c.events_len==1 && c.events[0]=='S');
  /* No resend even when send() returns just one of six bytes. */
  clear(&c); io.socket_selected=1; c.socket_return=0;
  assert(classic_write_data_core(&io,text,6)==0);
  assert(c.events_len==1 && c.events[0]=='S');
  clear(&c);io.socket_selected=0; io.dump_enabled=1;
  c.stdout_return=6;c.dump_return=0;
  assert(classic_write_data_core(&io,text,6)==1);
  assert(c.events_len==2 && c.events[0]=='D' && c.events[1]=='O');
  clear(&c);io.stdout_enabled=0;io.dump_enabled=1;c.dump_return=6;
  assert(classic_write_data_core(&io,text,6)==0);
  assert(c.events_len==1 && c.events[0]=='D');
  puts("classic WriteData original branch behavior checks passed");
  return 0;
}
