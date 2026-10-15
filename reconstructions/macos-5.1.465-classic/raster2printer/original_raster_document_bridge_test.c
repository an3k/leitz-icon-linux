#define _POSIX_C_SOURCE 200809L
#include "original_raster_document_bridge.h"
#include "original_raster_page_lifecycle.h"
#include "original_raster_process_page_adapter.h"
#include "original_raster_dither_bridge.h"
#include "original_raster_connected_job.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cups_raster_s {unsigned header_calls,pixel_calls;};
typedef struct {
    classic_original_raster_document_bridge bridge; /* MUST BE FIRST */
    classic_original_page_lifecycle_ops page_io;
    classic_original_process_page_adapter process_io;
    struct cups_raster_s raster;
    char events[128];unsigned events_len;
    uint8_t payload[512];size_t bytes;
    unsigned write_lengths[20],write_calls,close_calls,flush_calls;
    unsigned pages,queries;
    uint32_t job_size;
    int terminated,high_res,cut,ppd_fails,process_error,rgb;
    unsigned optimization_mode,dither_calls;
} mock;
static mock *active;
static void event(mock*m,char c) {
    assert(m->events_len<sizeof(m->events)-1);
    m->events[m->events_len++]=c;m->events[m->events_len]=0;
}
unsigned cupsRasterReadHeader2(cups_raster_t *r,cups_page_header2_t *h) {
    assert(active&&r==&active->raster);mock*m=active;event(m,'H');
    ++m->raster.header_calls;
    if(m->raster.header_calls>1)return 0;
    memset(h,0,sizeof *h);h->cupsWidth=8;h->cupsHeight=1;
    h->cupsBytesPerLine=m->rgb?24u:1u;
    h->cupsBitsPerPixel=m->rgb?24u:1u;
    h->cupsColorSpace=m->rgb?1u:0u;
    strcpy(h->cupsPageSizeName,"OEM_MEDIA");
    return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t *r,unsigned char *dst,unsigned n) {
    mock*m=active;assert(r==&m->raster && dst && n==(m->rgb?24u:1u));
    event(m,'X');++m->raster.pixel_calls;
    if(m->rgb) memset(dst,0,24); /* RGB24 black -> OEM 0xff packed row */
    else dst[0]=0x5a;
    return n;
}
static int file_open(void*c,const char*n,int f){(void)c;(void)n;(void)f;assert(0);return -1;}
static void snooze(void*c,unsigned n){(void)c;(void)n;assert(0);}
static void *raster_open(void*c,int fd,int mode) {
    mock*m=c;event(m,'R');assert(fd==0&&mode==0);return &m->raster;
}
static void *ppd_open(void*c,const char*s) {
    mock*m=c;event(m,'P');assert(s&&strcmp(s,"classic.ppd")==0);
    return m->ppd_fails?NULL:m;
}
static int parse(void*c,const char*s,int n,classic_oem_cups_option**p) {
    mock*m=c;event(m,'A');assert(!strcmp(s,"test")&&!n);
    *p=NULL;return 0;
}
static void defaults(void*c,void*p){mock*m=c;event(m,'D');assert(p==m);}
static int mark_one(void*c,void*p,const char*a,const char*b){
    (void)c;(void)p;(void)a;(void)b;assert(0);return 0;
}
static int mark_all(void*c,void*p,int n,classic_oem_cups_option*o){
    mock*m=c;event(m,'L');assert(p==m&&!n&&o==NULL);return 0;
}
static const char *choice(void*c,void*p,const char*name){
    mock*m=c;assert(p==m);event(m,'Q');
    const char *names[]={"Resolution","PrintQuality","DebugMode",
      "PrintThroughPRN","OptimizationLevel","LabelCutter"};
    assert(m->queries<6 && !strcmp(name,names[m->queries]));
    ++m->queries;
    if(!strcmp(name,"Resolution")&&m->high_res)return "600dpi";
    if(!strcmp(name,"PrintQuality")&&m->rgb)return "Bayer8x8";
    if(!strcmp(name,"LabelCutter")&&m->cut)return "ON";
    if(!strcmp(name,"OptimizationLevel")){
        if(m->optimization_mode==2)return "2";
        if(m->optimization_mode==3)return "3";
    }
    return NULL;
}
static void freeopts(void*c,int n,classic_oem_cups_option*o){
    mock*m=c;event(m,'F');assert(n==0&&o==NULL);
}
static void closeppd(void*c,void*p){mock*m=c;event(m,'p');assert(p==m);}
static void opendebug(void*c){event(c,'B');}
static void stdout_enable(void*c,int on){assert(on==1);event(c,'T');}
static void dither(void*c,classic_oem_read_descriptor*d,const uint8_t*in,
                   uint8_t*out,unsigned q){
    mock*m=c;event(m,'d');assert(m->rgb&&d->cups_color_space==1);
    assert(d->cups_bytes_per_line==24 && q==0);
    ++m->dither_calls;
    classic_original_raster_dither_bridge(c,d,in,out,q);
    assert(out[0]==0xff);
}
static void startpage(void*c,classic_oem_read_descriptor*d){
    mock*m=c;event(m,'G');assert(d->cups_raster==&m->raster);
    classic_original_start_page(&m->page_io,d);
}
static int process(void*c,classic_oem_read_descriptor*d,const uint8_t*buf,
                   unsigned i,int reprint) {
    mock*m=c;event(m,'Y');assert(d->cups_raster==&m->raster);
    assert(buf[0]==(m->rgb?0xff:0x5a)&&i==0&&!reprint);++m->pages;
    if(m->optimization_mode==2 || m->optimization_mode==3)
        return classic_original_process_page_callback(
            &m->process_io,d,buf,i,reprint);
    return m->process_error;
}
static void endpage(void*c,const void*desc,size_t n) {
    mock*m=c;event(m,'E');assert(n==0x10a8);
    const classic_oem_read_descriptor*d=desc;
    assert(d->cups_raster==&m->raster);
    classic_original_end_page(&m->page_io,d);
}
static int terminated(void*c){return ((mock*)c)->terminated;}
static int doc_write(void*c,const uint8_t*buf,size_t n,
                     const void*d,size_t ds,int sock){
    mock*m=c;event(m,'W');assert(ds==0x10a8&&sock==-1);
    assert(m->write_calls<20&&m->bytes+n<=sizeof m->payload);
    void *handle=NULL;memcpy(&handle,d,sizeof handle);
    assert(handle==&m->raster); /* OEM handle visible before StartDocument */
    m->write_lengths[m->write_calls++]=(unsigned)n;
    memcpy(m->payload+m->bytes,buf,n);m->bytes+=n;
    return -1; /* real OEM ignores failed WriteData return */
}
static void doc_flush(void*c) {mock*m=c;event(m,'U');++m->flush_calls;}
static void doc_close(void*c,void*p) {
    mock*m=c;event(m,'C');assert(p==&m->raster);++m->close_calls;
}
static int close_fd(void*c,int f){(void)c;(void)f;assert(0);return -1;}
static void close_debug(void*c){event(c,'b');}
static int send_prn(void*c){(void)c;assert(0);return 0;}
static classic_original_raster_connected_io setup(mock*m) {
    classic_original_raster_connected_io io={0};
    classic_oem_preflight_io p={m,file_open,snooze,raster_open,ppd_open,
                               parse,defaults,mark_one,mark_all};
    classic_oem_option_initialization_io init={
        p,choice,freeopts,closeppd,opendebug,stdout_enable,
        classic_original_bridge_start_document};
    classic_oem_page_output_io page={m,dither,startpage,process,endpage};
    classic_original_raster_finish_ops finish={
        m,classic_original_bridge_end_document,close_fd,close_debug,
        send_prn,doc_write};
    classic_original_document_ops doc={
        m,doc_write,doc_flush,doc_close,&m->job_size,-1,terminated};
    m->bridge.original=doc;
    m->page_io=(classic_original_page_lifecycle_ops){m,doc_write,doc_flush,&m->job_size,-1};
    classic_original_process_page_adapter_initialize(
        &m->process_io,m->page_io,terminated,m);
    io.initialization=init;io.page_output=page;
    io.finish=finish;io.is_terminated=terminated;io.termination_context=m;
    io.job_size=&m->job_size;io.socket_fd=-1;return io;
}
/* This fixture launches multiple independently modeled CUPS jobs in one
 * Unix test process. Real CUPS starts a fresh filter process for each job,
 * with the OEM ProcessPage function-local static 32-bit width initialized
 * by loader BSS to zero each time. Only the TEST must emulate that fresh
 * process boundary; production adapter initialization does NOT clear it.
 */
static void reset(mock*m){
    memset(m,0,sizeof *m);
    classic_original_mode2_static_width=0;
    active=m;
}
int main(void) {
    assert(setenv("PPD","classic.ppd",1)==0);
    char *argv[]={"raster2printer","1","u","title","1","test",NULL};
    classic_oem_read_descriptor descriptor;
    classic_oem_active_page_state page;
    unsigned index=99;
    FILE *f=tmpfile();assert(f);
    int fd=dup(fileno(stderr));assert(fd>=0);
    assert(dup2(fileno(f),fileno(stderr))>=0);
    mock m;reset(&m);
    classic_original_raster_connected_io io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(index==1 && m.pages==1 && m.raster.header_calls==2);
    assert(m.raster.pixel_calls==1 && m.write_calls==5);
    assert(m.write_lengths[0]==122 && m.write_lengths[1]==11);
    assert(m.write_lengths[2]==2 && m.write_lengths[3]==2 && m.write_lengths[4]==4);
    assert(m.job_size==141 && m.bytes==141 && m.flush_calls==2);
    assert(m.close_calls==1 && descriptor.cups_raster==&m.raster);
    assert(!page.page_buffer&&!page.dithered_page);
    for(unsigned i=0;i<121;i++)assert(m.payload[i]==0x1b);
    assert(m.payload[121]=='@');
    assert(!memcmp(m.payload+122,"\033L\0\037\033w\0\010\033B\0",11));
    assert(!memcmp(m.payload+133,"\033G",2));
    assert(!memcmp(m.payload+135,"\033E\033@\033Z",6));
    assert(strstr(m.events,"GWYEWUH")!=NULL); /* page flush before EOF read */
    assert(strstr(m.events,"WUW")!=NULL); /* conditional EndDocument flush */
    assert(m.events[m.events_len-1]=='b'); /* no extra ESC at job size 141 */

    reset(&m);m.high_res=1;m.cut=1;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(m.write_calls==5 && m.write_lengths[0]==122);
    assert(m.write_lengths[1]==2 && m.write_lengths[2]==11);
    assert(m.write_lengths[3]==4 && m.write_lengths[4]==4);
    assert(m.payload[122]==0x1b && m.payload[123]=='i');
    assert(!memcmp(m.payload+124,"\033L\0\036\033w\0\010\033B\0",11));
    assert(!memcmp(m.payload+135,"\033G\033E\033@\033Z",8));
    assert(m.job_size==143 && m.bytes==143 && m.flush_calls==1 && m.close_calls==1);

    reset(&m);m.terminated=1;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(index==0&&m.raster.header_calls==0&&m.pages==0);
    assert(m.job_size==132&&m.bytes==132&&m.write_calls==3);
    assert(m.write_lengths[0]==122&&m.write_lengths[1]==4&&m.write_lengths[2]==6);
    assert(!memcmp(m.payload+126,"\033@\033Z\0\0",6));
    assert(m.flush_calls==0&&m.close_calls==1);

    reset(&m);m.ppd_fails=1;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==-1);
    assert(m.write_calls==0&&m.close_calls==0&&m.queries==0);
    assert(descriptor.cups_raster==&m.raster); /* OEM leaked stream on PPD error */

    reset(&m);io=setup(&m);
    assert(classic_original_raster_connected_job(&io,4,argv,&descriptor,&page,&index)==1);
    assert(m.events_len==0&&m.write_calls==0&&m.close_calls==0);


    /* End-to-end original normal ProcessPage mode 2 (raw-byte branch):
     * the command encoder, separate OEM WriteData calls and page/document
     * lifecycle now execute in one joined C job. */
    reset(&m);m.optimization_mode=2;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(index==1 && m.pages==1 && m.close_calls==1);
    assert(m.write_calls==9 && m.job_size==149 && m.bytes==149);
    assert(m.flush_calls==2 && m.process_io.cache.mode2_previous_width==1);
    assert(m.write_lengths[0]==122 && m.write_lengths[1]==11);
    assert(m.write_lengths[2]==3 && m.write_lengths[3]==3);
    assert(m.write_lengths[4]==1 && m.write_lengths[5]==1);
    assert(m.write_lengths[6]==2 && m.write_lengths[7]==2 &&
           m.write_lengths[8]==4);
    assert(!memcmp(m.payload+133,"\033B\0\033D\001\026\132",8));
    assert(!memcmp(m.payload+141,"\033G\033E\033@\033Z",8));

    /* Mode 3: no ESC B when left offset starts at zero; the original
     * raw marker and its one data byte are a SINGLE WriteData call. */
    reset(&m);m.optimization_mode=3;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(index==1 && m.pages==1 && m.close_calls==1);
    assert(m.write_calls==7 && m.job_size==146 && m.bytes==146);
    assert(m.flush_calls==2 && m.process_io.cache.mode3_previous_width==1);
    assert(m.write_lengths[0]==122 && m.write_lengths[1]==11);
    assert(m.write_lengths[2]==3 && m.write_lengths[3]==2);
    assert(m.write_lengths[4]==2 && m.write_lengths[5]==2 &&
           m.write_lengths[6]==4);
    assert(!memcmp(m.payload+133,"\033D\001\026\132",5));
    assert(!memcmp(m.payload+138,"\033G\033E\033@\033Z",8));

    /* RGB24 native path: actual OEM _DitherPage, followed by recovered
     * mode-2 _ProcessPage and all original page/document boundaries.
     * No independently invented RGB converter is used. */
    reset(&m);m.rgb=1;m.optimization_mode=2;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&descriptor,&page,&index)==0);
    assert(index==1 && m.pages==1 && m.dither_calls==1);
    assert(m.raster.pixel_calls==1 && m.close_calls==1);
    assert(m.write_calls==9 && m.job_size==149 && m.bytes==149);
    assert(m.flush_calls==2 && descriptor.cups_color_space==1);
    assert(descriptor.packed_bytes_per_line==1);
    assert(m.write_lengths[2]==3 && m.write_lengths[3]==3);
    assert(m.write_lengths[4]==1 && m.write_lengths[5]==1);
    assert(!memcmp(m.payload+133,"\033B\0\033D\001\026\377",8));
    assert(!memcmp(m.payload+141,"\033G\033E\033@\033Z",8));
    assert(strstr(m.events,"XdG")!=NULL);

    assert(fflush(stderr)==0);
    assert(dup2(fd,fileno(stderr))>=0);close(fd);fclose(f);
    puts("Connected OEM document and page lifecycle, raster output, flush and epilogue passed");
    return 0;
}
