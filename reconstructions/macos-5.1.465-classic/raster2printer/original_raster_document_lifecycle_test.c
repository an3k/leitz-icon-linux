#define _POSIX_C_SOURCE 200809L
#include "original_raster_document_lifecycle.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned char descriptor[CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE];
    uint8_t payload[256];
    size_t size;
    size_t offsets[8];
    size_t lengths[8];
    unsigned writes,flushes,closes;
    void *closed_handle;
    int sock,terminate,change_socket_after_first;
    unsigned socket_reads;
    uint32_t job_size;
    char order[16];
    size_t order_len;
} mock;
static void event(mock*m,char x) {
    assert(m->order_len < sizeof(m->order)-1);
    m->order[m->order_len++]=x;
    m->order[m->order_len]='\0';
}
static int write_data(void *ctx,const uint8_t *p,size_t n,
                      const void *desc,size_t len,int fd) {
    mock*m=ctx;event(m,'W');
    assert(fd==m->sock && len==sizeof m->descriptor);
    assert(desc!=m->descriptor);
    assert(memcmp(desc,m->descriptor,len)==0);
    assert(m->writes<8 && m->size+n<=sizeof m->payload);
    m->offsets[m->writes]=m->size;
    m->lengths[m->writes]=n;
    ++m->writes;
    if(m->change_socket_after_first && m->writes==1)m->sock=29;
    memcpy(m->payload+m->size,p,n);
    m->size+=n;
    return -1; /* Manufacturer ignores WriteData errors. */
}
static void flush(void*ctx){mock*m=ctx;event(m,'F');++m->flushes;}
static void close_raster(void*ctx,void*handle){
    mock*m=ctx;event(m,'C');++m->closes;m->closed_handle=handle;
}
static int terminated(void*ctx){return ((mock*)ctx)->terminate;}
static int socket_now(void*ctx){mock*m=ctx;++m->socket_reads;return m->sock;}
static classic_original_document_ops setup(mock*m) {
    classic_original_document_ops io={m,write_data,flush,close_raster,
                                      &m->job_size,m->sock,terminated,NULL};
    return io;
}
static void begin(mock*m,int high_res,int cut,int term,unsigned initial) {
    memset(m,0,sizeof *m);
    uint32_t h=(uint32_t)high_res,k=(uint32_t)cut;
    memcpy(m->descriptor+0x14,&h,4);
    memcpy(m->descriptor+0x10,&k,4);
    void *handle=m; memcpy(m->descriptor,&handle,sizeof handle);
    m->terminate=term;m->job_size=initial;m->sock=13;
}
int main(void) {
    mock m;begin(&m,0,0,0,1);
    classic_original_document_ops io=setup(&m);
    classic_original_start_document(&io,m.descriptor);
    assert(m.job_size==123 && m.writes==1 && m.size==122);
    assert(strcmp(m.order,"W")==0 && m.lengths[0]==122);
    for(unsigned i=0;i<121;++i)assert(m.payload[i]==0x1b);
    assert(m.payload[121]=='@');

    begin(&m,1,0,0,100);io=setup(&m);
    classic_original_start_document(&io,m.descriptor);
    assert(m.job_size==224 && m.writes==2 && m.size==124);
    assert(strcmp(m.order,"WW")==0);
    assert(m.lengths[0]==122 && m.lengths[1]==2 && m.offsets[1]==122);
    assert(m.payload[122]==0x1b && m.payload[123]=='i');
    assert(m.flushes==0 && m.closes==0);

    begin(&m,0,0,0,20);io=setup(&m);
    classic_original_end_document(&io,m.descriptor);
    assert(m.job_size==26 && m.writes==2 && m.flushes==1 && m.closes==1);
    assert(strcmp(m.order,"WFWC")==0);
    assert(m.lengths[0]==2 && m.lengths[1]==4);
    assert(!memcmp(m.payload,"\033E\033@\033Z",6));
    assert(m.closed_handle==&m);
    void *after=(void *)1;
    memcpy(&after,m.descriptor,sizeof after);assert(after==NULL);
    /* OEM does not zero the other descriptor fields. */
    assert(m.descriptor[0x14]==0);

    begin(&m,0,1,0,7);io=setup(&m);
    classic_original_end_document(&io,m.descriptor);
    assert(m.job_size==11 && m.writes==1 && m.flushes==0 && m.closes==1);
    assert(strcmp(m.order,"WC")==0 && m.lengths[0]==4);
    assert(!memcmp(m.payload,"\033@\033Z",4));

    begin(&m,0,0,1,0);io=setup(&m);
    classic_original_end_document(&io,m.descriptor);
    assert(m.job_size==4 && m.writes==1 && m.flushes==0 && m.closes==1);
    assert(strcmp(m.order,"WC")==0);
    assert(!memcmp(m.payload,"\033@\033Z",4));

    /* OEM calls cupsRasterClose unconditionally, even for a NULL handle.
     * The test-only callback records NULL; it does not call CUPS. */
    begin(&m,0,1,0,9);io=setup(&m);
    memset(m.descriptor,0,sizeof(void*));
    classic_original_end_document(&io,m.descriptor);
    assert(m.closed_handle==NULL && m.closes==1);
    /* OEM native reads socket for each StartDocument WriteData. */
    begin(&m,1,0,0,0);io=setup(&m);
    m.change_socket_after_first=1;io.current_socket_fd=socket_now;
    classic_original_start_document(&io,m.descriptor);
    assert(m.writes==2 && m.socket_reads==2 && m.sock==29);
    assert(m.size==124 && m.job_size==124);
    /* EndDocument independently reloads before both sends too. */
    begin(&m,0,0,0,0);io=setup(&m);
    m.change_socket_after_first=1;io.current_socket_fd=socket_now;
    classic_original_end_document(&io,m.descriptor);
    assert(m.writes==2 && m.socket_reads==2 && m.sock==29);
    assert(strcmp(m.order,"WFWC")==0);
    puts("OEM native StartDocument/EndDocument live socket and I/O passed");
    return 0;
}
