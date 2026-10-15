#define _POSIX_C_SOURCE 200809L
#include "original_raster_process_page_modes01.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    classic_oem_read_descriptor descriptor;
    unsigned calls;
    uint32_t job_size;
    size_t length[72], offset[72], written;
    unsigned char payload[640];
    unsigned stop_after_calls;
    int sent_wrong_descriptor;
    int live_socket,change_socket_after_first;
    unsigned socket_reads;
} mock;

static int send_bytes(void *ctx,const uint8_t *data,size_t n,
                      const void *desc,size_t desc_len,int fd) {
    mock*m=ctx;
    assert(fd==m->live_socket);
    assert(desc_len==sizeof m->descriptor);
    assert(desc!=&m->descriptor);
    assert(memcmp(desc,&m->descriptor,desc_len)==0);
    assert(m->calls<72 && m->written+n<=sizeof m->payload);
    m->offset[m->calls]=m->written;
    m->length[m->calls++]=n;
    memcpy(m->payload+m->written,data,n);
    m->written+=n;
    if(m->change_socket_after_first && m->calls==1)m->live_socket=19;
    return -1; /* OEM ignores WriteData errors. */
}
static int socket_now(void*ctx){mock*m=ctx;++m->socket_reads;return m->live_socket;}
static void unused_flush(void*ctx){(void)ctx;assert(0);}
static int is_terminated(void *ctx) {
    mock*m=ctx;
    return m->stop_after_calls && m->calls>=m->stop_after_calls;
}
static classic_original_process_modes01_io setup(mock *m){
    classic_original_process_modes01_io io={0};
    io.output=(classic_original_page_lifecycle_ops){
        m,send_bytes,unused_flush,&m->job_size,-1,NULL
    };
    io.is_terminated=is_terminated;
    io.termination_context=m;
    return io;
}
static void initialize(mock*m,unsigned mode,unsigned stride,unsigned height){
    memset(m,0,sizeof *m);
    m->live_socket=-1;
    m->descriptor.packed_bytes_per_line=stride;
    m->descriptor.cups_height=(uint16_t)height;
    uint32_t v=mode;
    memcpy((uint8_t*)&m->descriptor+0x40,&v,sizeof v);
}
int main(void) {
    mock m;
    uint8_t pixels[32]={0};
    for(unsigned i=0;i<16;++i)pixels[i]=(uint8_t)(i+1u);
    initialize(&m,0,3,4);
    classic_original_process_modes01_io io=setup(&m);
    FILE*cap=tmpfile();assert(cap);
    int save=dup(fileno(stderr));assert(save>=0);
    assert(dup2(fileno(cap),fileno(stderr))>=0);
    assert(classic_original_process_page_modes01(&io,&m.descriptor,pixels,0)==0);
    assert(m.calls==5 && m.job_size==32 && m.written==32);
    assert(m.length[0]==4 && !memcmp(m.payload,"\033D\003\026",4));
    for(unsigned y=0;y<4;++y){
        assert(m.length[y+1]==7);
        assert(!memcmp(m.payload+4+7*y,pixels+3*y,7));
    }
    assert(classic_original_process_page_modes01(&io,&m.descriptor,pixels,1)==0);
    assert(m.calls==10 && m.job_size==64);

    initialize(&m,1,3,4);io=setup(&m);
    memset(pixels,0,sizeof pixels);
    pixels[7]=0x25; /* row 0 blank, row 1 blank, row 2 nonwhite */
    assert(classic_original_process_page_modes01(&io,&m.descriptor,pixels,3)==0);
    assert(m.calls==3 && m.job_size==15 && m.written==15);
    assert(m.length[0]==4 && !memcmp(m.payload,"\033D\003\026",4));
    assert(m.length[1]==4 && !memcmp(m.payload+4,"\033f\0\002",4));
    assert(m.length[2]==7 && !memcmp(m.payload+8,pixels+6,7));

    initialize(&m,1,3,4);io=setup(&m);
    memset(pixels,0,sizeof pixels);
    assert(classic_original_process_page_modes01(&io,&m.descriptor,pixels,0)==0);
    assert(m.calls==1 && m.job_size==4); /* trailing blanks not flushed */

    initialize(&m,0,1,64);io=setup(&m);
    uint8_t rows[68]={0};
    assert(classic_original_process_page_modes01(&io,&m.descriptor,rows,0)==0);
    assert(m.calls==65 && m.job_size==324);
    assert(m.length[1]==5 && m.length[64]==5);

    initialize(&m,0,1,64);m.stop_after_calls=2;io=setup(&m);
    assert(classic_original_process_page_modes01(&io,&m.descriptor,rows,0)==0);
    assert(m.calls==2 && m.job_size==9); /* SIGTERM after first raster row */

    /* Original reloads the socket for the mode-0 preamble and both
     * subsequent row writes. Four lookahead bytes remain readable. */
    initialize(&m,0,1,2);io=setup(&m);
    m.change_socket_after_first=1;io.output.current_socket_fd=socket_now;
    assert(classic_original_process_page_modes01(
        &io,&m.descriptor,rows,0)==0);
    assert(m.calls==3 && m.socket_reads==3 && m.live_socket==19);
    initialize(&m,3,1,64);io=setup(&m);
    assert(classic_original_process_page_modes01(&io,&m.descriptor,rows,0)==-2);
    assert(m.calls==0 && m.job_size==0); /* no mode-3 behavior invented */

    assert(fflush(stderr)==0);
    assert(dup2(save,fileno(stderr))>=0 && close(save)==0);
    rewind(cap);
    char text[4096]={0};size_t got=fread(text,1,sizeof(text)-1,cap);
    text[got]=0;assert(fclose(cap)==0);
    assert(strstr(text,"\nINFO: Printing page 1, 0% complete...")!=NULL);
    assert(strstr(text,"\nATTR: job-media-progress=0")!=NULL);
    assert(strstr(text,"\nINFO: Printing page 1, 50% complete...")!=NULL);
    assert(strstr(text,"\nATTR: job-media-progress=50")!=NULL);
    assert(strstr(text,"\nINFO: Printing page 4, 0% complete...")!=NULL);
    puts("OEM ProcessPage modes 0/1 dispatch, progress, termination, write and row bytes passed");
    return 0;
}
