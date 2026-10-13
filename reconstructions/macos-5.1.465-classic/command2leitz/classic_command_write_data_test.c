#include "classic_command_write_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {unsigned sent,dumped,written;ssize_t socket_result;size_t output_result;} state;
static ssize_t send_once(void *v,int socket,const void *p,size_t n) {
    state *s=v;(void)p;assert(socket==7 && n==6);++s->sent;return s->socket_result;
}
static size_t dump_once(void *v,const void *p,size_t n) {
    state *s=v;(void)p;assert(n==6);++s->dumped;return 0;
}
static size_t stdout_once(void *v,const void *p,size_t n) {
    state *s=v;(void)p;assert(n==6);++s->written;return s->output_result;
}
int main(void) {
    state s={0};
    classic_command_output io={&s,1,7,1,1,send_once,dump_once,stdout_once};
    s.socket_result=1;
    assert(classic_command_write_data(&io,"abcdef",6)==1);
    assert(s.sent==1 && s.dumped==0 && s.written==0);
    s.socket_result=0;
    assert(classic_command_write_data(&io,"abcdef",6)==0 && s.sent==2);
    io.socket_fd=-1;s.output_result=1;
    assert(classic_command_write_data(&io,"abcdef",6)==1);
    assert(s.dumped==1 && s.written==1);
    io.stdout_present=0;
    assert(classic_command_write_data(&io,"abcdef",6)==0);
    assert(s.dumped==2 && s.written==1);
    assert(classic_command_write_data(&io,"abcdef",0)==0);
    assert(s.dumped==2 && s.written==1);
    puts("command2leitz original WriteData sink-branch tests passed");
    return 0;
}
