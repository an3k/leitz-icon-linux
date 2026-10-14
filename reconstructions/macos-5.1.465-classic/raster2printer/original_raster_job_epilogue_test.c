#define _POSIX_C_SOURCE 200809L
#include "original_raster_job_epilogue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char order[32]; size_t used;
    unsigned close_count, prn_count, bytes_count;
    int closed[2];
    uint8_t bytes[8];
    uint8_t expect_descriptor[CLASSIC_OEM_DESCRIPTOR_BYTES];
} mock;
static void step(mock *m,char c){assert(m->used<sizeof m->order-1);m->order[m->used++]=c;m->order[m->used]=0;}
static void check_copy(mock *m,const void*p,size_t n){assert(n==CLASSIC_OEM_DESCRIPTOR_BYTES);assert(memcmp(p,m->expect_descriptor,n)==0);assert(p!=m->expect_descriptor);}
static void end_document(void*ctx,const void*d,size_t n){mock*m=ctx;step(m,'E');check_copy(m,d,n);}
static int close_fd(void*ctx,int fd){mock*m=ctx;step(m,'C');assert(m->close_count<2);m->closed[m->close_count++]=fd;return -1;}
static void close_dbg(void*ctx){step(ctx,'D');}
static int send_prn(void*ctx){mock*m=ctx;step(m,'P');m->prn_count++;return -1;}
static int write_data(void*ctx,const uint8_t*d,size_t n,const void*descriptor,size_t len,int sock){
    mock*m=ctx;step(m,'W');assert(sock==11);check_copy(m,descriptor,len);
    assert(m->bytes_count+n<=sizeof m->bytes);
    memcpy(m->bytes+m->bytes_count,d,n);m->bytes_count+=(unsigned)n;
    return 0; /* OEM ignores _WriteData return */
}
static classic_original_raster_finish_ops callbacks(mock *m){
    classic_original_raster_finish_ops out={m,end_document,close_fd,close_dbg,send_prn,write_data};return out;
}
static void setup_desc(mock*m,uint8_t *buf,int prn,const char *uri){
    memset(buf,0,CLASSIC_OEM_DESCRIPTOR_BYTES);
    unsigned n=(unsigned)prn;memcpy(buf+8,&n,4);
    strcpy((char*)buf+0x50,uri);
    memcpy(m->expect_descriptor,buf,CLASSIC_OEM_DESCRIPTOR_BYTES);
}
static void check_run(mock*m,uint8_t*desc,unsigned *job_size,
                      int fd,int sock,int resolved,int term,int rc,
                      const char *seq,const char *stderr_expected){
    classic_original_raster_finish_ops io=callbacks(m);
    uint64_t pointer_slot = resolved ? 0x123456789abcdef0ULL : 0;
    memcpy(desc+0x1090,&pointer_slot,sizeof pointer_slot);
    memcpy(m->expect_descriptor,desc,CLASSIC_OEM_DESCRIPTOR_BYTES);
    classic_original_raster_finish_state state={desc,fd,sock,term,rc,job_size};
    FILE *capture=tmpfile();assert(capture);
    int saved=dup(fileno(stderr));assert(saved>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);
    assert(classic_original_raster_finish_job(&io,&state)==rc);
    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0);assert(close(saved)==0);
    rewind(capture);char text[1024]={0};size_t r=fread(text,1,sizeof text-1,capture);
    text[r]=0;assert(fclose(capture)==0);
    if(strcmp(text,stderr_expected)!=0){fprintf(stderr,"expected: [%s]\nactual: [%s]\n",stderr_expected,text);abort();}
    assert(strcmp(m->order,seq)==0);
}
int main(void){
    mock m={0};uint8_t d[CLASSIC_OEM_DESCRIPTOR_BYTES];unsigned bytes=65;
    setup_desc(&m,d,0,"usb://Icon");
    check_run(&m,d,&bytes,0,11,0,0,7,"ED",
      "\nraster2printer.c:444:raster2printer(): do not print through PRN file "
      "\nSTATE: none\n\nraster2printer.c:471:raster2printer():  job size : 65 "
      "\nraster2printer.c:472:raster2printer(): raster filter finished\n");
    assert(bytes==65&&m.close_count==0&&m.prn_count==0&&m.bytes_count==0);

    memset(&m,0,sizeof m);setup_desc(&m,d,1,"ipp://Icon");bytes=58;
    check_run(&m,d,&bytes,9,11,1,1,-3,"ECDPCWW",
      "\nraster2printer.c:440:raster2printer(): print through PRN file "
      "\nSTATE: none\n\nraster2printer.c:471:raster2printer():  job size : 64 "
      "\nraster2printer.c:472:raster2printer(): raster filter finished\n");
    assert(bytes==64 && m.close_count==2 && m.closed[0]==9&&m.closed[1]==11);
    assert(m.prn_count==1 && m.bytes_count==7);
    assert(memcmp(m.bytes,"\033@\033Z\0\0\033",7)==0);

    memset(&m,0,sizeof m);setup_desc(&m,d,0,"ipp://Icon");bytes=0;
    check_run(&m,d,&bytes,0,11,0,0,0,"EDW",
      "\nraster2printer.c:444:raster2printer(): do not print through PRN file "
      "\nSTATE: none\n\nraster2printer.c:471:raster2printer():  job size : 0 "
      "\nraster2printer.c:472:raster2printer(): raster filter finished\n");
    assert(m.bytes_count==1 && m.bytes[0]==0x1b && m.close_count==0);

    memset(&m,0,sizeof m);setup_desc(&m,d,0,"ipp://Icon");bytes=63;
    check_run(&m,d,&bytes,0,11,1,1,2,"EDCW",
      "\nraster2printer.c:444:raster2printer(): do not print through PRN file "
      "\nSTATE: none\n\nraster2printer.c:471:raster2printer():  job size : 69 "
      "\nraster2printer.c:472:raster2printer(): raster filter finished\n");
    assert(m.bytes_count==6 && m.closed[0]==11);
    puts("OEM job epilogue original call order, PRN/socket/termination/64-byte cases passed");
    return 0;
}
