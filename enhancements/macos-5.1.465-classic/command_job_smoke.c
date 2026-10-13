#include "mac_classic_command_job.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct { uint8_t bytes[8192];size_t count;size_t chunk; } sink;
static ssize_t transmit(void *p,const uint8_t *b,size_t n){
 sink *s=p;if(n>s->chunk)n=s->chunk;
 if(n>sizeof s->bytes-s->count)return -1;
 memcpy(s->bytes+s->count,b,n);s->count+=n;return (ssize_t)n;
}
typedef struct {uint8_t b[1024];ssize_t n;double timeout;} back;
static ssize_t response(void *v,uint8_t *b,size_t n,double seconds){
 back *a=v;a->timeout=seconds;
 if(a->n<0 || (size_t)a->n>n)return -1;
 memcpy(b,a->b,(size_t)a->n);return a->n;
}
typedef struct {char line[8][5000];size_t lines;} messages;
static int emit(void *v,const char *l){
 messages *m=v;if(m->lines>=8||strlen(l)>=sizeof m->line[0])return -1;
 strcpy(m->line[m->lines++],l);return 0;
}
typedef struct {unsigned calls;size_t count;} archive;
static int log_u_v(void *v,const char *name,const char *value,const uint8_t *p,size_t n){
 archive *a=v;(void)name;(void)value;(void)p;a->calls++;a->count=n;return 0;
}
int main(void){
 sink s={.chunk=1};mac_classic_output out={MAC_CLASSIC_STDOUT,transmit,&s,NULL,NULL};
 back b={.b={0x03},.n=1};mac_classic_status_io io={&out,response,&b};
 messages m={0};archive a={0};
 mac_classic_command_context c={&io,emit,&m,log_u_v,&a,7.0};
 assert(mac_classic_command_run(&c,"rEpOrTsTaTuS","")==1);
 assert(b.timeout==7.0 && s.count==4 && memcmp(s.bytes,"\033A\033Z",4)==0);
 assert(m.lines==2 && strstr(m.line[0],"STATE: -") &&
        !strcmp(m.line[1],"STATE: +com.leitz.ready"));
 s.count=0;m.lines=0;b.b[0]=0x30;
 assert(mac_classic_command_run(&c,"ReportStatus","")==1);
 assert(strstr(m.line[1],"paper-jam-error"));
 s.count=0;m.lines=0;b.n=0;
 assert(mac_classic_command_run(&c,"GetUnitedStatus","")==0);
 assert(s.count==2 && !memcmp(s.bytes,"\033U",2));
 assert(m.lines==1&&!strcmp(m.line[0],"ATTR: marker-message=NoData"));
 s.count=0;m.lines=0;b.n=-1;
 assert(mac_classic_command_run(&c,"GetFirmwareVersion","")==-1);
 assert(s.count==2 && !memcmp(s.bytes,"\033V",2));
 s.count=0;m.lines=0;b.n=4;
 b.b[0]='1';b.b[1]=0;b.b[2]='"';b.b[3]=0x1b;
 assert(mac_classic_command_run(&c,"GetFirmwareVersion","ver=")==1);
 assert(s.count==4 && !memcmp(s.bytes,"\033V\033Z",4));
 assert(m.lines==1 && !strcmp(m.line[0],"ATTR: marker-message=\"ver=1\\x00\\x22\\x1B\""));
 assert(a.calls==1 && a.count==4);
 assert(mac_classic_command_run(&c,"GetPrinterStatus","")==-1);
 assert(mac_classic_command_run(&c,"ReportState","")==-1);
 char rendered[20];assert(mac_classic_status_render(rendered,sizeof rendered,b.b,4)==0);
 assert(!strcmp(rendered,"1\\x00\\x22\\x1B"));
 char tight[3];assert(mac_classic_status_render(tight,sizeof tight,b.b,4)<0);
 b.n=1024;memset(b.b,1,sizeof b.b);s.count=0;m.lines=0;
 assert(mac_classic_command_run(&c,"GetUnitedStatus","")==1);
 assert(a.calls==2&&a.count==1024);
 puts("classic command job checks passed");
 return 0;
}
