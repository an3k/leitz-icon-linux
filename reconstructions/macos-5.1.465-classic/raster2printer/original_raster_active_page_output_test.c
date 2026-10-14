#define _POSIX_C_SOURCE 200809L
#include "original_raster_active_page_output.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char calls[32];
    unsigned n,quality,seen_page;
    int process_result;
    const uint8_t *selected;
    classic_oem_read_descriptor *descriptor;
} mock;
static void mark(mock *m,char c){assert(m->n<sizeof m->calls-1);m->calls[m->n++]=c;m->calls[m->n]=0;}
static void dither(void *v,classic_oem_read_descriptor *d,const uint8_t *in,uint8_t *out,unsigned q){
    mock*m=v;mark(m,'D');assert(d==m->descriptor);
    assert(in[0]==0x34 && out!=NULL);
    out[0]=0xf3;m->quality=q;
}
static void start(void *v,classic_oem_read_descriptor*d) {
    mock*m=v;mark(m,'S');assert(d==m->descriptor);
}
static int process(void*v,classic_oem_read_descriptor*d,const uint8_t *pixels,
                   unsigned page,int reprint) {
    mock*m=v;mark(m,'P');assert(d==m->descriptor && reprint==0);
    m->seen_page=page;m->selected=pixels;return m->process_result;
}
static void end(void*v,const void*desc,size_t length) {
    mock*m=v;mark(m,'E');
    assert(length==sizeof(classic_oem_read_descriptor));
    assert(memcmp(desc,m->descriptor,length)==0);
    assert(desc!=m->descriptor);
}
static classic_oem_page_output_io setup(mock*m){
    classic_oem_page_output_io io={m,dither,start,process,end};return io;
}
static void init_page(classic_oem_active_page_state*p,size_t n,int with_dither) {
    memset(p,0,sizeof *p);p->requested_bytes=(int32_t)n;
    p->page_buffer=malloc(n);assert(p->page_buffer);
    p->page_buffer[0]=0x34;
    if(with_dither) {p->dithered_page=malloc(n);assert(p->dithered_page);}
}
int main(void) {
    classic_oem_read_descriptor d={0};
    mock m={0};m.descriptor=&d;
    classic_oem_page_output_io io=setup(&m);
    classic_oem_active_page_state page={0};
    unsigned page_index=0;int process_result=-1;
    FILE *cap=tmpfile();assert(cap);
    int stderr_saved=dup(fileno(stderr));assert(stderr_saved>=0);
    assert(dup2(fileno(cap),fileno(stderr))>=0);

    init_page(&page,24,1);
    d.cups_color_space=1;
    unsigned quality=11;memcpy((unsigned char*)&d+0x4c,&quality,sizeof quality);
    assert(classic_original_raster_output_page(&io,&d,&page,&page_index,1,0,&process_result)==1);
    assert(strcmp(m.calls,"DSPE")==0 && m.quality==11 && m.seen_page==0);
    assert(m.selected!=NULL && page_index==1 && process_result==0);
    assert(page.page_buffer==NULL && page.dithered_page==NULL);

    memset(&m,0,sizeof m);m.descriptor=&d;io=setup(&m);
    init_page(&page,6,0);
    d.cups_color_space=0;
    m.process_result=7;
    assert(classic_original_raster_output_page(&io,&d,&page,&page_index,1,0,&process_result)==0);
    assert(strcmp(m.calls,"SPE")==0 && m.seen_page==1);
    assert(page_index==2 && process_result==7);
    assert(page.page_buffer==NULL && page.dithered_page==NULL);

    memset(&m,0,sizeof m);m.descriptor=&d;io=setup(&m);
    assert(classic_original_raster_output_page(&io,&d,&page,&page_index,1,0,&process_result)==0);
    assert(strcmp(m.calls,"")==0 && page_index==3 && process_result==1);

    /* At EOF or termination, the original exits the loop before this
     * per-page cleanup. Caller retains any pending buffer to clean later. */
    init_page(&page,4,1);
    assert(classic_original_raster_output_page(&io,&d,&page,&page_index,0,0,&process_result)==0);
    assert(page_index==3 && page.page_buffer!=NULL && page.dithered_page!=NULL);
    assert(classic_original_raster_output_page(&io,&d,&page,&page_index,1,1,&process_result)==0);
    assert(page_index==3 && page.page_buffer!=NULL && page.dithered_page!=NULL);
    free(page.page_buffer);free(page.dithered_page);

    assert(fflush(stderr)==0);
    assert(dup2(stderr_saved,fileno(stderr))>=0 && close(stderr_saved)==0);
    rewind(cap);char lines[4096]={0};size_t n=fread(lines,1,sizeof(lines)-1,cap);
    lines[n]=0;assert(fclose(cap)==0);
    assert(strstr(lines,
      "\nraster2printer.c:393:raster2printer(): \n shouldResumePrintingOfLastLabel = false"));
    unsigned printed=0;
    const char *q=lines;
    while((q=strstr(q,"\nSTATE: none\n"))!=NULL) {++printed;++q;}
    assert(printed==3);
    assert(strstr(lines,"\nPAGE: 1 1\n"));
    assert(strstr(lines,"\nPAGE: 2 1\n"));
    assert(strstr(lines,"\nPAGE: 3 1\n"));
    assert(strstr(lines,"\nINFO: pageBuffer null\n"));
    puts("OEM normal page dispatch, byte-copied EndPage, status and cleanup passed");
    return 0;
}
