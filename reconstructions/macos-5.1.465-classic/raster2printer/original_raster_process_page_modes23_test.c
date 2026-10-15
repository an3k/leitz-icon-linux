#define _POSIX_C_SOURCE 200809L
#include "original_raster_process_page_modes23.h"
#include "original_raster_process_page_adapter.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    classic_oem_read_descriptor descriptor;
    classic_mode23_job_cache cache;
    uint32_t job_size;
    size_t length[160], offsets[160], bytes;
    unsigned calls;
    uint8_t stream[2048];
    unsigned abort_after_calls;
} mock;
static int write_data(void *ctx,const uint8_t *data,size_t count,
                      const void *copy,size_t size,int fd){
    mock*m=ctx;assert(fd==-1);
    assert(size==sizeof m->descriptor && copy!=&m->descriptor);
    assert(memcmp(copy,&m->descriptor,size)==0);
    assert(m->calls<160 && m->bytes+count<=sizeof m->stream);
    m->length[m->calls]=count;m->offsets[m->calls]=m->bytes;
    ++m->calls;memcpy(m->stream+m->bytes,data,count);m->bytes+=count;
    return -1; /* OEM does not stop on failed WriteData. */
}
static void flush(void *ctx){(void)ctx;assert(0&&"No EndPage inside ProcessPage");}
static int term(void *ctx){
    mock*m=ctx;return m->abort_after_calls&&m->calls>=m->abort_after_calls;
}
static classic_original_process_modes23_io setup(mock*m){
    classic_original_process_modes23_io io={0};
    io.output=(classic_original_page_lifecycle_ops){
        m,write_data,flush,&m->job_size,-1
    };
    io.job_cache=&m->cache;
    io.is_terminated=term;io.termination_context=m;
    return io;
}
static void begin(mock*m,unsigned mode,unsigned stride,unsigned height){
    memset(m,0,sizeof *m);
    m->descriptor.packed_bytes_per_line=stride;
    m->descriptor.cups_height=(uint16_t)height;
    uint32_t op=mode;
    memcpy((uint8_t*)&m->descriptor+0x40,&op,sizeof op);
}
static void reset_output(mock*m){
    m->bytes=0;m->calls=0;m->job_size=0;
    memset(m->length,0,sizeof m->length);
}
static void check(const mock*m,const uint8_t*bytes,size_t n,
                  const size_t *parts,unsigned chunks){
    assert(m->bytes==n && m->job_size==n && m->calls==chunks);
    assert(memcmp(m->stream,bytes,n)==0);
    size_t total=0;
    for(unsigned i=0;i<chunks;++i){
        assert(m->offsets[i]==total);
        assert(m->length[i]==parts[i]);
        total+=parts[i];
    }
    assert(total==n);
}
int main(void){
    mock m;uint8_t pixels[24]={0};
    FILE *capture=tmpfile();assert(capture);
    int saved=dup(fileno(stderr));assert(saved>=0);
    assert(dup2(fileno(capture),fileno(stderr))>=0);

    begin(&m,2,4,4);
    uint8_t *r2=pixels+8,*r3=pixels+12;
    r2[0]=0;r2[1]=0xff;r2[2]=0xff;r2[3]=0xff;
    r3[0]=0;r3[1]=0x55;r3[2]=0x55;r3[3]=0;
    classic_original_process_modes23_io io=setup(&m);
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,0)==0);
    const uint8_t expected2[]={
        0x1b,'f',0,2,0x1b,'B',1,0x1b,'D',3,0x17,0x97,
        0x1b,'B',1,0x1b,'D',2,0x16,0x55,0x55};
    const size_t parts2[]={4,3,3,1,1,3,3,1,2};
    check(&m,expected2,sizeof expected2,parts2,9);
    assert(m.cache.mode2_previous_width==2);

    /* Mode-2 static width cache lives between page invocations.
     * ESC B is still emitted, ESC D is suppressed for unchanged width. */
    reset_output(&m);
    m.descriptor.cups_height=1;
    assert(classic_original_process_page_modes23(&io,&m.descriptor,r3,1)==0);
    const uint8_t cached2[]={0x1b,'B',1,0x16,0x55,0x55};
    const size_t cached2parts[]={3,1,2};
    check(&m,cached2,sizeof cached2,cached2parts,3);

    /* OEM fourth ProcessPage argument: reprinting the first nonblank
     * mode-2 row forces ESC D even when width is cached. A subsequent
     * ordinary page must not inherit that per-invocation flag. */
    reset_output(&m);
    io.reprint=1;
    assert(classic_original_process_page_modes23(&io,&m.descriptor,r3,2)==0);
    const uint8_t forced2[]={0x1b,'B',1,0x1b,'D',2,0x16,0x55,0x55};
    const size_t forced2parts[]={3,3,1,2};
    check(&m,forced2,sizeof forced2,forced2parts,4);
    io.reprint=0;
    reset_output(&m);
    assert(classic_original_process_page_modes23(&io,&m.descriptor,r3,3)==0);
    check(&m,cached2,sizeof cached2,cached2parts,3);

    /* Integration regression: the public ProcessPage dispatcher must
     * preserve cached width while forwarding the isolated reprint flag. */
    begin(&m,2,4,1);
    memset(pixels,0,sizeof pixels);
    pixels[1]=0x55;pixels[2]=0x55;
    io=setup(&m);
    classic_original_process_page_adapter adapter;
    classic_original_process_page_adapter_initialize(
        &adapter,io.output,term,&m);
    assert(classic_original_process_page_callback(
        &adapter,&m.descriptor,pixels,0,0)==0);
    check(&m,forced2,sizeof forced2,forced2parts,4);
    reset_output(&m);
    assert(classic_original_process_page_callback(
        &adapter,&m.descriptor,pixels,1,0)==0);
    check(&m,cached2,sizeof cached2,cached2parts,3);
    reset_output(&m);
    assert(classic_original_process_page_callback(
        &adapter,&m.descriptor,pixels,2,1)==0);
    check(&m,forced2,sizeof forced2,forced2parts,4);
    reset_output(&m);
    assert(classic_original_process_page_callback(
        &adapter,&m.descriptor,pixels,3,0)==0);
    check(&m,cached2,sizeof cached2,cached2parts,3);
    reset_output(&m);
    uint32_t unsupported=1;
    memcpy((uint8_t *)&m.descriptor+0x40,&unsupported,sizeof unsupported);
    assert(classic_original_process_page_callback(
        &adapter,&m.descriptor,pixels,4,1)==-2);
    assert(m.calls==0 && m.job_size==0);

    begin(&m,3,4,3);
    uint8_t *m0=pixels,*m1=pixels+4;
    memset(pixels,0,sizeof pixels);m0[2]=0xff;m1[2]=0xff;
    io=setup(&m);
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,2)==0);
    const uint8_t expected3[]={
        0x1b,'B',2,0x1b,'D',1,0x16,0xff,0x16,0xff};
    const size_t parts3[]={3,3,2,2};
    check(&m,expected3,sizeof expected3,parts3,4);
    assert(m.cache.mode3_previous_width==1);

    /* Page-local last-left-offset starts at zero on every new page;
     * the persistent width cache does not. */
    reset_output(&m);
    m.descriptor.cups_height=1;
    assert(classic_original_process_page_modes23(&io,&m.descriptor,m0,3)==0);
    const uint8_t cached3[]={0x1b,'B',2,0x16,0xff};
    const size_t cached3parts[]={3,2};
    check(&m,cached3,sizeof cached3,cached3parts,2);

    begin(&m,3,4,1);io=setup(&m);
    memset(pixels,0,sizeof pixels);pixels[0]=0xff;
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,0)==0);
    const uint8_t asym[]={
        0x1b,'D',4,0x16,0xff,0,0,0};
    const size_t asymparts[]={3,5};
    check(&m,asym,sizeof asym,asymparts,2);

    begin(&m,2,4,2);io=setup(&m);
    memset(pixels,0,sizeof pixels);
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,0)==0);
    assert(m.calls==0 && m.job_size==0); /* trailing deferred whites */

    begin(&m,2,4,4);io=setup(&m);
    memset(pixels,0,sizeof pixels);
    pixels[1]=0xff;pixels[2]=0xff;pixels[3]=0xff;
    m.abort_after_calls=4;
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,0)==0);
    assert(m.calls==4); /* check cancellation AFTER all writes for row */

    begin(&m,3,4,64);io=setup(&m);
    uint8_t white[256]={0};
    assert(classic_original_process_page_modes23(&io,&m.descriptor,white,0)==0);
    assert(m.calls==0);

    /* Exactly one fully completed 128-bit black run with more bits:
     * OEM source line 860 logs the resulting 0xff RLE byte once.
     * This must not change any of the emitted output bytes. */
    begin(&m,2,17,1);io=setup(&m);
    uint8_t full_ones[17];
    memset(full_ones,0xff,sizeof full_ones);
    assert(classic_original_process_page_modes23(
        &io,&m.descriptor,full_ones,0)==0);
    assert(m.job_size==9 && m.calls==4);

    /* OEM mode-0/1 paths do not use the fourth ProcessPage argument.
     * Exercise their actual public dispatcher with equal input and
     * compare normal vs explicit-reprint command bytes/call boundaries.
     * Supply four readable lookahead bytes for the OEM raw-row copy. */
    for (uint32_t mode=0; mode<=1; ++mode) {
        begin(&m,mode,4,1);
        io=setup(&m);
        classic_original_process_page_adapter_initialize(
            &adapter,io.output,term,&m);
        memset(pixels,0,sizeof pixels);
        pixels[0]=0xff;
        assert(classic_original_process_page_callback(
            &adapter,&m.descriptor,pixels,0,0)==0);
        assert(m.calls==2 && m.bytes==12 && m.job_size==12);
        const uint8_t *normal=m.stream;
        uint8_t expected[12];
        memcpy(expected,normal,sizeof expected);
        const size_t first_chunk=m.length[0];
        const size_t second_chunk=m.length[1];
        reset_output(&m);
        assert(classic_original_process_page_callback(
            &adapter,&m.descriptor,pixels,1,1)==0);
        assert(m.calls==2 && m.bytes==12 && m.job_size==12);
        assert(m.length[0]==first_chunk && m.length[1]==second_chunk);
        assert(memcmp(m.stream,expected,sizeof expected)==0);
    }

    begin(&m,1,4,1);io=setup(&m);
    assert(classic_original_process_page_modes23(&io,&m.descriptor,pixels,0)==-2);
    assert(m.calls==0 && m.job_size==0);

    assert(fflush(stderr)==0);
    assert(dup2(saved,fileno(stderr))>=0 && close(saved)==0);
    rewind(capture);
    char logs[4096]={0};size_t n=fread(logs,1,sizeof logs-1,capture);
    logs[n]=0;assert(fclose(capture)==0);
    assert(strstr(logs,
      "\nras" "ter2printer.c:860:ProcessPage(): first bit/byte. do not send 255"));
    assert(strstr(logs,"\nINFO: Printing page 1, 0% complete..."));
    assert(strstr(logs,"\nINFO: Printing page 1, 50% complete..."));
    assert(strstr(logs,"\nATTR: job-media-progress=50"));
    /* Manufacturer 0xff sentinel is initialized once per mode-2 row.
     * Three fixture rows have an initial set bit; long black runs do
     * NOT generate additional copies of this diagnostic. */
    const char *diag="raster2printer.c:860:ProcessPage(): first bit/byte. do not send 255";
    const char *cursor=logs;
    unsigned diag_count=0;
    while((cursor=strstr(cursor,diag))!=NULL) {
        ++diag_count;
        cursor+=strlen(diag);
    }
    assert(diag_count==3);
    assert(strstr(logs,"\nINFO: Printing page 3, 0% complete..."));
    puts("OEM mode-2/3 normal ProcessPage command boundaries, progress and state passed");
    return 0;
}
