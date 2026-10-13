#include "original_status_exchange.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char received[5][64];
    ssize_t received_sizes[5];
    unsigned read_index,calls,write_calls,reset_calls,sleep_calls;
    unsigned report_count,resolve_count,stop_after_reads;
    int last_errno,terminate;
    char reports[4096];
    unsigned char outbound[16];
    size_t out_count;
} mock;
static int term(void *p) {mock *m=p;return m->terminate || (m->stop_after_reads && m->read_index>=m->stop_after_reads);}
static int resolve(void *p){((mock*)p)->resolve_count++;return 1;}
static ssize_t consume(mock *m,void *dst,size_t capacity){
    assert(capacity==64);
    unsigned i=m->read_index++;
    assert(i<5);
    ssize_t n=m->received_sizes[i];
    if(n>0)memcpy(dst,m->received[i],(size_t)n);
    return n;
}
static ssize_t back(void *p,void *dst,size_t sz,double t){assert(t==7.0);return consume(p,dst,sz);}
static ssize_t sendv(void *p,int fd,const void *src,size_t sz){
    mock *m=p;assert(fd==9 && sz==2 && !memcmp(src,"\033U",2));
    m->write_calls++;return 2;
}
static ssize_t recvv(void *p,int fd,void *dst,size_t sz){assert(fd==9);return consume(p,dst,sz);}
static int put(void *p,int ch){mock *m=p;m->outbound[m->out_count++]=(unsigned char)ch;return ch;}
static int flush(void *p){((mock*)p)->calls++;return 0;}
static void note(void *p,const char *s){
    mock *m=p;size_t used=strlen(m->reports);
    assert(used+strlen(s)+1<sizeof m->reports);
    strcat(m->reports,s);m->report_count++;
}
static void write_job(void *p,const void *data,size_t n){
    mock *m=p;assert(n==4 && !memcmp(data,"\033@\033Z",4));m->reset_calls++;
}
static void sleepv(void *p,unsigned seconds){mock *m=p;assert(seconds==5);m->sleep_calls++;}
static int last_error(void *p){return ((mock*)p)->last_errno;}
static classic_original_status_io setup(mock *m,unsigned *size,const char *uri,int socket,int resolved){
    classic_original_status_io io={m,uri,socket,resolved,term,resolve,
                                   sendv,recvv,put,flush,back,note,
                                   write_job,sleepv,size,last_error};
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
    assert(m.read_index==2 && m.out_count==4);

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

    memset(&m,0,sizeof m);io=setup(&m,&size,"usb://leitz",-1,0);
    m.received_sizes[0]=64;
    assert(classic_original_check_status(&io)==-2); /* undefined original */
    puts("Original _CheckPrinterStatus defined I/O path checks passed");
    return 0;
}
