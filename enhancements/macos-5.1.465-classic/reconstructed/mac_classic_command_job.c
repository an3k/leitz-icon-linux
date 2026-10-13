#define _POSIX_C_SOURCE 200809L
#include "mac_classic_command_job.h"
#include "mac_classic_command_dispatch.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

ssize_t mac_classic_command_write_fd(void *ctx,const uint8_t *p,size_t n){
  if(!ctx || (!p && n))return -1;
  int fd=*(int *)ctx;
  ssize_t done;
  do { done=write(fd,p,n); } while(done<0 && errno==EINTR);
  /* Higher layer resumes only after a reported partial write. */
  return done;
}
int mac_classic_command_emit_stderr(void *ctx,const char *line){
  FILE *out=(FILE *)ctx;
  if(!out||!line)return -1;
  return fprintf(out,"%s\n",line)<0 ? -1:0;
}
int mac_classic_status_render(char *dst,size_t cap,const uint8_t *data,size_t n){
  if(!dst||!cap||(!data&&n))return -1;
  size_t pos=0;
  static const char hex[]="0123456789ABCDEF";
  for(size_t i=0;i<n;i++){
    uint8_t b=data[i];
    if(b>=0x20 && b<=0x7e && b!='\\' && b!='"'){
      if(pos+1>=cap)return -1;
      dst[pos++]=(char)b;
    }else{
      if(pos+4>=cap)return -1;
      dst[pos++]='\\';dst[pos++]='x';
      dst[pos++]=hex[b>>4];dst[pos++]=hex[b&15];
    }
  }
  dst[pos]=0;
  return 0;
}
static int emit(mac_classic_command_context *ctx,const char *line){
  if(!ctx->emit || ctx->emit(ctx->emit_context,line))return -1;
  return 0;
}
int mac_classic_command_run(mac_classic_command_context *ctx,
                             const char *name,const char *value){
  if(!ctx||!ctx->status_io||!name||!ctx->emit)return -1;
  uint8_t command;
  if(!mac_classic_command_select(name,&command))return -1;
  mac_classic_status_result r;
  int state=mac_classic_status_query(ctx->status_io,command,ctx->timeout,&r);
  if(state<=0){
    if(emit(ctx,"ATTR: marker-message=NoData"))return -1;
    return state;
  }
  if(command=='A'){
    if(emit(ctx,mac_classic_state_remove_a()))return -1;
    const char *additional=mac_classic_state_add_a(r.status_a);
    if(additional && emit(ctx,additional))return -1;
  }else{
    /* Original writes response plus its command-value argument to
     * /private/var/log/leitz.log and posts notify_post before emitting
     * marker-message. The hook is opt-in, and never changes host files
     * without an explicitly configured archive path. */
    if(ctx->archive_u_v && ctx->archive_u_v(ctx->archive_context,name,
                        value?value:"",r.reply,(size_t)r.reply_length))return -1;
    char response[4*MAC_CLASSIC_STATUS_MAX_BYTES+1];
    char line[sizeof(response)+512];
    if(mac_classic_status_render(response,sizeof response,r.reply,
                                (size_t)r.reply_length))return -1;
    const char *prefix=value?value:"";
    size_t prefix_len=strlen(prefix);
    if(prefix_len>96)return -1;
    char escaped_value[96*4+1];
    if(mac_classic_status_render(escaped_value,sizeof escaped_value,
        (const uint8_t *)prefix,prefix_len))return -1;
    int n=snprintf(line,sizeof line,"ATTR: marker-message=\"%s%s\"",
                   escaped_value,response);
    if(n<0||(size_t)n>=sizeof line)return -1;
    if(emit(ctx,line))return -1;
  }
  return 1;
}
