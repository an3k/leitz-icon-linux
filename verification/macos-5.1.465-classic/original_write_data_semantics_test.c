#define _POSIX_C_SOURCE 200809L
#include "../../reconstructions/macos-5.1.465-classic/command2leitz/classic_command_write_data.h"
#include "../../reconstructions/macos-5.1.465-classic/raster2printer/classic_write_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    ssize_t socket_return;
    size_t fwrite_return;
    unsigned sent, dumped, stdout;
} state;
static ssize_t send_data(void *v,int fd,const void *buf,size_t n){
    state*s=v;(void)buf;assert(fd==7&&n==6);s->sent++;return s->socket_return;
}
static ssize_t send_raster(void *v,const void*buf,size_t n){
    return send_data(v,7,buf,n);
}
static size_t stdout_data(void*v,const void*buf,size_t n){
    state*s=v;(void)buf;assert(n==6);s->stdout++;return s->fwrite_return;
}
static size_t dump_data(void*v,const void*buf,size_t n){
    state*s=v;(void)buf;assert(n==6);s->dumped++;return 0;
}
static void verify(int raster){
    state st={0};
    classic_command_output ci={&st,1,7,1,1,send_data,dump_data,stdout_data};
    classic_write_data_io ri={&st,1,1,1,send_raster,stdout_data,dump_data};
#define RUN(N) (raster ? classic_write_data_core(&ri,"ABCDEF",(N)) : classic_command_write_data(&ci,"ABCDEF",(N)))
    FILE* captured=tmpfile();assert(captured);
    int fd=dup(fileno(stderr));assert(fd>=0);
    assert(dup2(fileno(captured),fileno(stderr))>=0);
    /* The OEM compares the signed socket result with ZERO, not > 0. */
    st.socket_return=-1;assert(RUN(6)==1);
    st.socket_return=1;assert(RUN(6)==1);
    st.socket_return=0;assert(RUN(6)==0);
    ci.ipp_selected=0;ri.socket_selected=0;
    st.fwrite_return=0;assert(RUN(6)==0);
    st.fwrite_return=1;assert(RUN(6)==1);
    assert(RUN(0)==0); /* early exit, no dump, no error message */
    ci.stdout_present=0;ri.stdout_enabled=0;assert(RUN(6)==0);
    assert(st.sent==3&&st.stdout==2&&st.dumped==3);
    assert(fflush(stderr)==0);
    assert(dup2(fd,fileno(stderr))>=0);close(fd);
    rewind(captured);char err[400]={0};
    size_t n=fread(err,1,sizeof(err)-1,captured);err[n]=0;
    assert(fclose(captured)==0);
    const char *line="\nraster_c_functions.c:55:WriteData(): an error occured while writing data of size : 6";
    char expected[400];snprintf(expected,sizeof expected,"%s%s",line,line);
    assert(strcmp(err,expected)==0);
#undef RUN
}
int main(void){
    verify(0);verify(1);
    puts("OEM _WriteData signed-result and diagnostics parity checks passed");
    return 0;
}
