#define _POSIX_C_SOURCE 200809L
#include "original_raster_connected_job.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cups_raster_s {unsigned headers,reads;};
typedef struct {
    struct cups_raster_s source;
    char order[128];unsigned order_count;
    unsigned pages, page_read_count, dither_calls, end_document_calls;
    unsigned query_count, closed;
    int process_result,terminate,zero_pixels;
    unsigned char sent[8];size_t sent_bytes;
    uint32_t job_size;
    unsigned last_printed_index;
} mock;
static mock *active;
static void mark(mock*m,char ch){assert(m->order_count<127);m->order[m->order_count++]=ch;m->order[m->order_count]=0;}
unsigned cupsRasterReadHeader2(cups_raster_t*r,cups_page_header2_t*h){
    assert(active && r==&active->source);
    mock*m=active;mark(m,'H');++m->source.headers;
    if(m->source.headers>2 || (m->zero_pixels && m->source.headers>1)) return 0;
    memset(h,0,sizeof *h);
    h->cupsHeight=2;h->cupsWidth=8;h->cupsMediaType=3;
    h->cupsColorSpace=(m->source.headers==1?0:1);
    h->cupsBitsPerPixel=(m->source.headers==1?1:24);
    h->cupsBytesPerLine=(m->source.headers==1?1:24);
    strcpy(h->cupsPageSizeName,"OEM_CUSTOM");
    return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t*r,unsigned char*d,unsigned size){
    assert(r==&active->source && d);
    mock*m=active;mark(m,'X');++m->source.reads;
    assert(size==(m->source.reads==1?2u:48u));
    if(m->zero_pixels)return 0;
    memset(d,0x53,size);
    return size; /* no short reads are needed for this connector test */
}
static int open_input(void*ctx,const char*name,int mode){(void)ctx;(void)name;(void)mode;assert(0&&"stdin test");return -1;}
static void sleeper(void*ctx,unsigned n){(void)ctx;(void)n;assert(0);}
static void *open_raster(void*ctx,int fd,int mode){mock*m=ctx;mark(m,'R');assert(fd==0&&mode==0);return &m->source;}
static void *open_ppd(void*ctx,const char*name){mock*m=ctx;mark(m,'P');assert(name && strcmp(name,"classic.ppd")==0);return m;}
static int parse_options(void*ctx,const char*str,int base,classic_oem_cups_option**opts){
    mock*m=ctx;mark(m,'A');assert(!strcmp(str,"PrintQuality=none")&&base==0);
    *opts=NULL;return 0;
}
static void defaults(void*ctx,void*ppd){mock*m=ctx;mark(m,'D');assert(ppd==m);}
static int mark_one(void*ctx,void*ppd,const char*n,const char*v){
    (void)ctx;(void)ppd;(void)n;(void)v;assert(0);return 0;
}
static int mark_all(void*ctx,void*ppd,int n,classic_oem_cups_option*opts){
    mock*m=ctx;mark(m,'L');assert(ppd==m&&n==0&&opts==NULL);return 0;
}
static const char* choice(void*ctx,void*ppd,const char*name) {
    mock*m=ctx;assert(ppd==m);mark(m,'Q');m->query_count++;
    static const char* keys[]={"Resolution","PrintQuality","DebugMode","PrintThroughPRN","OptimizationLevel","LabelCutter"};
    assert(m->query_count<=6 && strcmp(name,keys[m->query_count-1])==0);
    return NULL;
}
static void free_options(void*ctx,int n,classic_oem_cups_option*opts){
    mock*m=ctx;mark(m,'F');assert(n==0&&opts==NULL);
}
static void close_ppd(void*ctx,void*ppd){mock*m=ctx;assert(ppd==m);mark(m,'C');}
static void debug_open(void*ctx){mark(ctx,'B');}
static void choose_stdout(void*ctx,int enabled){assert(enabled==1);mark(ctx,'T');}
static void startdoc(void*ctx,const unsigned char *bytes,unsigned n){
    mock*m=ctx;mark(m,'S');assert(n==0x10a8);assert(bytes[0]==0); /* descriptor initially zero */
}
static void dither(void*ctx,classic_oem_read_descriptor*d,const uint8_t*src,uint8_t*out,unsigned q){
    mock*m=ctx;mark(m,'I');assert(d&&src&&out&&q==0);assert(src[0]==0x53);
    out[0]=0xa5;++m->dither_calls;
}
static void startpage(void*ctx,classic_oem_read_descriptor*d){
    mock*m=ctx;mark(m,'G');assert(d->cups_raster==&m->source);
}
static int processpage(void*ctx,classic_oem_read_descriptor*d,const uint8_t*bytes,unsigned idx,int reprint){
    mock*m=ctx;mark(m,'Y');assert(reprint==0 && idx==m->pages);
    assert(bytes[0]==(idx==0?0x53:0xa5));
    assert(d->cups_raster==&m->source);
    m->last_printed_index=idx;++m->pages;
    return m->process_result;
}
static void endpage(void*ctx,const void*desc,size_t len){
    mock*m=ctx;mark(m,'E');assert(len==0x10a8);
    const classic_oem_read_descriptor *d=desc;
    assert(d->cups_raster==&m->source);
}
static int terminated(void*ctx){return ((mock*)ctx)->terminate;}
static void end_document(void*ctx,const void*desc,size_t len){
    mock*m=ctx;mark(m,'Z');assert(len==0x10a8);
    const classic_oem_read_descriptor *d=desc;
    assert(d->cups_raster==&m->source);++m->end_document_calls;
}
static int closefd(void*ctx,int fd){mock*m=ctx;mark(m,'c');m->closed++;(void)fd;return 0;}
static void close_debug(void*ctx){mark(ctx,'b');}
static int send_prn(void*ctx){(void)ctx;assert(0);return 0;}
static int write_data(void*ctx,const uint8_t*d,size_t n,const void*desc,size_t len,int sock){
    mock*m=ctx;mark(m,'W');assert(len==0x10a8 && sock==-1 && desc);
    assert(m->sent_bytes+n<=sizeof m->sent);
    memcpy(m->sent+m->sent_bytes,d,n);m->sent_bytes+=n;return -1;/* OEM ignores */
}
int classic_raster_quality_selector(const char *name){(void)name;assert(0);return -1;}
static classic_original_raster_connected_io setup(mock*m){
    classic_original_raster_connected_io io={0};
    classic_oem_preflight_io pf={m,open_input,sleeper,open_raster,open_ppd,
                                parse_options,defaults,mark_one,mark_all};
    classic_oem_option_initialization_io init={pf,choice,free_options,
                                               close_ppd,debug_open,choose_stdout,startdoc};
    classic_oem_page_output_io output={m,dither,startpage,processpage,endpage};
    classic_original_raster_finish_ops finish={m,end_document,closefd,
                                               close_debug,send_prn,write_data};
    io.initialization=init;io.page_output=output;io.finish=finish;
    io.is_terminated=terminated;io.termination_context=m;
    io.socket_fd=-1;io.job_size=&m->job_size;
    return io;
}
static void init_mock(mock*m){memset(m,0,sizeof *m);m->job_size=65;active=m;}
int main(void){
    assert(setenv("PPD","classic.ppd",1)==0);
    char *argv[]={"raster2printer","1","user","title","1","PrintQuality=none",NULL};
    classic_oem_read_descriptor desc;
    classic_oem_active_page_state page;
    unsigned index=123;

    FILE *captured=tmpfile();assert(captured);
    int saved=dup(fileno(stderr));assert(saved>=0);
    assert(dup2(fileno(captured),fileno(stderr))>=0);
    mock m;init_mock(&m);
    classic_original_raster_connected_io io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&desc,&page,&index)==0);
    assert(m.pages==2&&m.dither_calls==1&&m.query_count==6);
    assert(m.source.headers==3&&m.source.reads==2);
    assert(m.end_document_calls==1&&index==2&&page.page_number==2);
    assert(page.page_buffer==NULL&&page.dithered_page==NULL);
    assert(m.closed==0&&m.sent_bytes==0);
    assert(strstr(m.order,"GYE")!=NULL && strstr(m.order,"IGYE")!=NULL);
    assert(m.order[m.order_count-2]=='Z' && m.order[m.order_count-1]=='b');

    /* ProcessPage failure: stop after the first page and pass its result
     * through the already reconstructed original EndDocument epilogue. */
    init_mock(&m);m.process_result=7;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&desc,&page,&index)==7);
    assert(index==1&&m.pages==1&&m.source.headers==1&&m.end_document_calls==1);
    assert(!page.page_buffer&&!page.dithered_page);

    /* Original early argc return: no page, status query or end-document. */
    init_mock(&m);io=setup(&m);
    assert(classic_original_raster_connected_job(&io,4,argv,&desc,&page,&index)==1);
    assert(m.order_count==0&&m.end_document_calls==0);

    /* Pre-existing termination is passed into the original epilogue;
     * manufacturer abort write is 6 bytes, plus ESC when job size=64. */
    init_mock(&m);m.terminate=1;m.job_size=58;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&desc,&page,&index)==0);
    assert(m.source.headers==0&&m.end_document_calls==1&&index==0);
    assert(m.job_size==64&&m.sent_bytes==7);
    assert(memcmp(m.sent,"\033@\033Z\0\0\033",7)==0);

    /* The independently recovered OEM inline reader leaves a live buffer
     * after a zero pixel return. Test-only ownership cleanup is explicit. */
    init_mock(&m);m.zero_pixels=1;io=setup(&m);
    assert(classic_original_raster_connected_job(&io,6,argv,&desc,&page,&index)==0);
    assert(m.source.headers==1&&m.source.reads==1&&m.pages==0);
    assert(m.end_document_calls==1&&page.page_buffer);
    free(page.page_buffer);page.page_buffer=NULL;

    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0);assert(close(saved)==0);
    rewind(captured);char stderr_text[8192]={0};
    size_t n=fread(stderr_text,1,sizeof(stderr_text)-1,captured);
    stderr_text[n]=0;assert(fclose(captured)==0);
    /* Native OEM _raster2printer assembly at 0x100005523 / 0x1000055c3
     * writes both messages after StartDocument, and loop head
     * 0x10000567e logs the current index even before EOF. */
    unsigned start_256_count=0,start_258_count=0,non_reprint_count=0;
    const char *found=stderr_text;
    while((found=strstr(found,
        "\nraster2printer.c:256:raster2printer(): started printing pages"))!=NULL) {
        ++start_256_count; ++found;
    }
    found=stderr_text;
    while((found=strstr(found,
        "\nraster2printer.c:258:raster2printer(): started printing pages"))!=NULL) {
        ++start_258_count; ++found;
    }
    found=stderr_text;
    while((found=strstr(found,"\n Not reprinted i_page = "))!=NULL) {
        ++non_reprint_count; ++found;
    }
    /* Valid scenarios: normal 2-page, failed ProcessPage, early SIGTERM,
     * zero-pixel read. Invalid-argc scenario never reaches StartDocument.
     * Normal 2-page run makes a third read attempt that sees EOF. */
    assert(start_256_count==4 && start_258_count==4);
    assert(non_reprint_count==5);
    assert(strstr(stderr_text,
        "\nraster2printer.c:256:raster2printer(): started printing pages"
        "\nraster2printer.c:258:raster2printer(): started printing pages"
        "\n Not reprinted i_page = 0.\n")!=NULL);
    assert(strstr(stderr_text,"\n Not reprinted i_page = 1.\n")!=NULL);
    assert(strstr(stderr_text,"\n Not reprinted i_page = 2.\n")!=NULL);
    assert(strstr(stderr_text,"PAGE: 2 1\n")!=NULL);
    assert(strstr(stderr_text,"raster2printer.c:472:raster2printer(): raster filter finished\n")!=NULL);
    puts("Connected OEM raster startup/read/dispatch/epilogue C paths passed");
    return 0;
}
