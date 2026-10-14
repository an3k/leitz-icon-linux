#include "original_status_exchange.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char received[5][64];
    ssize_t received_sizes[5];
    unsigned read_index,calls,write_calls,reset_calls,sleep_calls;
    unsigned report_count,resolve_count,stop_after_reads;
    int last_errno,terminate;
    int live_socket,live_resolved;
    unsigned socket_reads,resolved_reads;
    int drop_socket_after_sleep;
    char reports[4096];
    unsigned char outbound[16];
    size_t out_count;
    unsigned char descriptor[CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE];
    int last_write_fd;
} mock;
static int term(void *p) {mock *m=p;return m->terminate || (m->stop_after_reads && m->read_index>=m->stop_after_reads);}
static int resolve(void *p){((mock*)p)->resolve_count++;return 1;}
static int dynamic_socket(void *p){
    mock *m=p;
    ++m->socket_reads;
    return m->live_socket;
}
static int dynamic_resolved(void *p){
    mock *m=p;
    ++m->resolved_reads;
    return m->live_resolved;
}
static ssize_t consume(mock *m,void *dst,size_t capacity){
    assert(capacity==64);
    unsigned i=m->read_index++;
    assert(i<5);
    ssize_t n=m->received_sizes[i];
    if(n>0)memcpy(dst,m->received[i],(size_t)n);
    return n;
}
static ssize_t back(void *p,void *dst,size_t sz,double t){assert(t==7.0);return consume(p,dst,sz);}
static ssize_t sendv(void *p,int fd,const void *src,size_t sz,int flags){
    mock *m=p;assert(fd==9 && flags==0 && sz==2 && !memcmp(src,"\033U",2));
    m->write_calls++;return 2;
}
static ssize_t recvv(void *p,int fd,void *dst,size_t sz,int flags){assert(fd==9 && flags==0);return consume(p,dst,sz);}
static int put(void *p,int ch){mock *m=p;m->outbound[m->out_count++]=(unsigned char)ch;return ch;}
static int flush(void *p){((mock*)p)->calls++;return 0;}
static void note(void *p,const char *s){
    mock *m=p;size_t used=strlen(m->reports);
    assert(used+strlen(s)+1<sizeof m->reports);
    strcat(m->reports,s);m->report_count++;
}
static int write_job(void *p,const uint8_t *data,size_t n,
                     const void *descriptor,size_t descriptor_size,int socket_fd){
    mock *m=p;
    assert(n==4 && !memcmp(data,"\033@\033Z",4));
    assert(descriptor_size==CLASSIC_OEM_DOCUMENT_DESCRIPTOR_SIZE);
    assert(descriptor!=m->descriptor);
    assert(!memcmp(descriptor,m->descriptor,descriptor_size));
    m->last_write_fd=socket_fd;
    ++m->reset_calls;
    return -1; /* Original ignores failed _WriteData on reset. */
}
static void sleepv(void *p,unsigned seconds){
    mock *m=p;assert(seconds==5);m->sleep_calls++;
    if(m->drop_socket_after_sleep)m->live_socket=-1;
}
static int last_error(void *p){return ((mock*)p)->last_errno;}
static classic_original_status_io setup(mock *m,unsigned *size,const char *uri,int socket,int resolved){
    for(size_t i=0;i<sizeof m->descriptor;++i)
        m->descriptor[i]=(unsigned char)(i*37u+11u);
    classic_original_status_io io={m,uri,socket,resolved,term,resolve,
                                   sendv,recvv,put,flush,back,note,
                                   write_job,sleepv,size,last_error,m->descriptor,NULL,NULL};
    return io;
}
static void response(mock *m,unsigned i,const char *text){
    size_t n=strlen(text);assert(n<64);memcpy(m->received[i],text,n);
    m->received_sizes[i]=(ssize_t)n;
}
int main(void){
    mock m={0};unsigned size=11;
    classic_original_status_io io=setup(&m,&size,"usb://leitz",-1,0);
    response(&m,0,"SKU:OK;");
    assert(classic_original_check_status(&io)==0);
    assert(m.read_index==1 && m.out_count==2 && !memcmp(m.outbound,"\033U",2));
    assert(strstr(m.reports,"STATE: +com.leitz.ready")!=NULL && size==11);

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    response(&m,0,"USED:-1");response(&m,1,"SKU:OK;");
    assert(classic_original_check_status(&io)==1);
    assert(m.sleep_calls==1 && m.reset_calls==1 && size==15);
    assert(m.last_write_fd==-1 && m.read_index==2 && m.out_count==4);

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    response(&m,0,"SKU:Door Open;");response(&m,1,"SKU:No Cartridge;");
    m.stop_after_reads=2;
    assert(classic_original_check_status(&io)==1);
    assert(m.read_index==2 && m.sleep_calls==2 && m.reset_calls==0);

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    m.received_sizes[0]=0;assert(classic_original_check_status(&io)==0);
    assert(strstr(m.reports,"INFO: empty back channel data")!=NULL && m.sleep_calls==0);

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    m.received_sizes[0]=-1;m.last_errno=5;assert(classic_original_check_status(&io)==0);
    assert(strstr(m.reports,"errno=5")!=NULL && m.sleep_calls==0);

    memset(&m,0,sizeof m);io=setup(&m,&size,"ipp://leitz",-1,0);
    assert(classic_original_check_status(&io)==0);
    assert(m.resolve_count==1 && m.read_index==0 && m.out_count==0);

    memset(&m,0,sizeof m);io=setup(&m,&size,"ipp://leitz",9,1);
    response(&m,0,"SKU:OK;");assert(classic_original_check_status(&io)==0);
    assert(m.write_calls==1 && m.out_count==0 && m.read_index==1);

    /* OEM IPP reset passes the same by-value descriptor and native fd
     * after a two-byte ESC U socket send and 64-byte recv. */
    memset(&m,0,sizeof m);io=setup(&m,&size,"ipp://leitz",9,1);
    response(&m,0,"SKU:Load Error;");response(&m,1,"SKU:OK;");
    assert(classic_original_check_status(&io)==1);
    assert(m.write_calls==2 && m.reset_calls==1 && m.last_write_fd==9);
    assert(m.sleep_calls==1 && size==19 && m.read_index==2);
    assert(m.out_count==0);

    /* OEM re-reads private sock on every retry, not just at entry:
     * the first reply requests a retry and the sleep mock makes the
     * current socket unavailable. The second loop must call the
     * original resolver rather than send again on an old descriptor.
     * The initial 9 in the static field is intentionally stale. */
    memset(&m,0,sizeof m);io=setup(&m,&size,"ipp://leitz",9,1);
    m.live_socket=9;m.live_resolved=1;m.drop_socket_after_sleep=1;
    io.current_socket_fd=dynamic_socket;
    io.resolved_uri_is_present=dynamic_resolved;
    response(&m,0,"SKU:Door Open;");
    assert(classic_original_check_status(&io)==1);
    assert(m.sleep_calls==1 && m.resolve_count==1);
    assert(m.read_index==1 && m.write_calls==1);
    assert(m.live_socket==-1 && m.socket_reads>=3);
    assert(m.resolved_reads==1);

    /* On each new call the native global getters override stale
     * positional snapshot fields, with no invented URI resolution. */
    memset(&m,0,sizeof m);io=setup(&m,&size,"ipp://leitz",-1,0);
    m.live_socket=9;m.live_resolved=1;
    io.current_socket_fd=dynamic_socket;
    io.resolved_uri_is_present=dynamic_resolved;
    response(&m,0,"SKU:OK;");
    assert(classic_original_check_status(&io)==0);
    assert(m.read_index==1 && m.write_calls==1);
    assert(m.resolve_count==0 && m.resolved_reads==1);

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    m.received_sizes[0]=64;
    assert(classic_original_check_status(&io)==-2); /* undefined original */
    puts("Original _CheckPrinterStatus defined I/O path checks passed");
    return 0;
}
