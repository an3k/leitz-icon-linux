#include "original_raster_page_lifecycle.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    classic_oem_read_descriptor original;
    uint8_t writes[32];
    size_t offsets[5], lengths[5],total;
    unsigned count,flushes;
    char operations[10];
    size_t operation_count;
    uint32_t job_size;
    int sock,change_socket_after_first;
    unsigned socket_reads;
} fixture;
static void op(fixture*f,char c){
    assert(f->operation_count<sizeof f->operations-1);
    f->operations[f->operation_count++]=c;
    f->operations[f->operation_count]=0;
}
static int write_bytes(void *context,const uint8_t *data,size_t len,
                       const void *descriptor,size_t size,int fd) {
    fixture*f=context;op(f,'W');
    assert(fd==f->sock && size==sizeof f->original);
    assert(descriptor != &f->original);
    assert(memcmp(descriptor,&f->original,size)==0);
    assert(f->count<5 && f->total+len<=sizeof f->writes);
    f->offsets[f->count]=f->total;f->lengths[f->count]=len;
    ++f->count;memcpy(f->writes+f->total,data,len);f->total+=len;
    if(f->change_socket_after_first && f->count==1)f->sock=19;
    return -1; /* OEM intentionally ignores failed WriteData. */
}
static void flush_data(void *context){
    fixture*f=context;op(f,'F');++f->flushes;
}
static int socket_now(void*ctx){fixture*f=ctx;++f->socket_reads;return f->sock;}
static classic_original_page_lifecycle_ops io(fixture*f) {
    classic_original_page_lifecycle_ops ops={f,write_bytes,flush_data,&f->job_size,7,NULL};
    return ops;
}
static void options(fixture*f,unsigned height,unsigned width,int hires,int cut){
    memset(f,0,sizeof *f);
    f->sock=7;
    f->original.cups_height=(uint16_t)height;
    f->original.cups_width=(uint16_t)width;
    uint32_t h=(uint32_t)hires,c=(uint32_t)cut;
    memcpy((uint8_t*)&f->original+0x14,&h,4);
    memcpy((uint8_t*)&f->original+0x10,&c,4);
}
int main(void) {
    fixture f;
    options(&f,100,704,0,0);f.job_size=33;
    classic_original_page_lifecycle_ops o=io(&f);
    classic_original_page_start_callback(&o,&f.original);
    assert(f.count==1 && f.lengths[0]==11 && f.job_size==44);
    assert(!memcmp(f.writes,"\033L\000\202\033w\002\300\033B\000",11));
    classic_original_page_end_callback(&o,&f.original,sizeof f.original);
    assert(f.count==2 && f.lengths[1]==2 && f.job_size==46);
    assert(!memcmp(f.writes+11,"\033G",2));
    assert(f.flushes==1 && !strcmp(f.operations,"WWF"));

    options(&f,100,704,1,1);o=io(&f);
    classic_original_start_page(&o,&f.original);
    classic_original_end_page(&o,&f.original);
    assert(f.count==2 && f.job_size==15 && f.flushes==1);
    assert(f.lengths[0]==11 && f.lengths[1]==4);
    assert(!memcmp(f.writes,"\033L\000\120\033w\002\300\033B\000",11));
    assert(!memcmp(f.writes+11,"\033G\033E",4));

    /* OEM first truncates height + DPI lead-in to uint16_t, *then*
     * applies high-resolution division, cf. 0x1000073d8..7405. */
    options(&f,65535,65535,0,0);o=io(&f);
    classic_original_start_page(&o,&f.original);
    assert(f.writes[2]==0 && f.writes[3]==29);
    assert(f.writes[6]==0xff && f.writes[7]==0xff);
    options(&f,65535,65535,1,0);o=io(&f);
    classic_original_start_page(&o,&f.original);
    assert(f.writes[2]==0 && f.writes[3]==29);
    assert(f.count==1 && f.flushes==0);

    options(&f,0,0,1,1);o=io(&f);
    classic_original_page_end_callback(&o,&f.original,sizeof f.original);
    assert(f.job_size==4 && f.count==1 && f.flushes==1);
    options(&f,100,704,0,0);o=io(&f);
    f.change_socket_after_first=1;o.current_socket_fd=socket_now;
    classic_original_start_page(&o,&f.original);
    classic_original_end_page(&o,&f.original);
    assert(f.count==2 && f.socket_reads==2 && f.sock==19);
    assert(f.job_size==13 && f.flushes==1);
    puts("OEM StartPage/EndPage live socket and output parity passed");
    return 0;
}
