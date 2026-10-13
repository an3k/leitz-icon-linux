/*
 * Native CUPS 2.x shell for the separately reconstructed classic macOS
 * raster job. This is NEW integration C, not the original manufacturer
 * _main ABI or a 1:1 reconstructed executable. The supported path is
 * the ordinary stdout CUPS raster stream and the OEM's deferred PRN file
 * transfer mode. The IPP network route remains incomplete.
 */
#include "classic_raster_job_recovered.h"
#include "raster2printer_quality_recovered.h"
#include "original_prn_helpers.h"
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static volatile sig_atomic_t stop_requested=0;
static void handle_term(int n) {(void)n;stop_requested=1;}

typedef struct {cups_raster_t *handle;cups_page_header2_t original;} input;
static unsigned read_header(void *v,classic_page_header *out) {
    input *p=v;
    if(!cupsRasterReadHeader2(p->handle,&p->original)) return 0;
    out->cups_width=p->original.cupsWidth;
    out->cups_height=p->original.cupsHeight;
    out->cups_bytes_per_line=p->original.cupsBytesPerLine;
    out->cups_bits_per_pixel=p->original.cupsBitsPerPixel;
    out->cups_color_space=p->original.cupsColorSpace;
    return 1;
}
static unsigned read_pixels(void *v,void *buf,unsigned n) {
    return cupsRasterReadPixels(((input*)v)->handle,buf,n);
}
static int emit_stdout(void *unused,const uint8_t *buf,size_t n) {
    (void)unused;
    /* Follow OEM one-call positive-result acceptance for its normal
     * stdout fwrite path; do not retry short writes. */
    return fwrite(buf,n,1,stdout)>0 ? 0 : -1;
}
static void flush_stdout(void *unused) {(void)unused;fflush(stdout);}
static int emit_prn(void *context,const uint8_t *buf,size_t n) {
    classic_original_prn_state *state=context;
    /* The original writes the dump once and ignores a short or failed
     * fwrite. With __fd_stdout=NULL it does not send directly to stdout. */
    classic_original_prn_write_buffer(state,buf,n);
    return 0;
}
static void flush_prn(void *context) {
    classic_original_prn_state *state=context;
    /* OEM flushes stdout even when deferred mode has disabled its
     * _WriteData stdout channel. It ignores fflush return codes. */
    fflush(stdout);
    if(state->dump) fflush(state->dump);
}
static void progress_stdout(void *unused,unsigned page,unsigned row,
                            unsigned percent) {
    (void)unused;(void)row;
    fprintf(stderr,"INFO: Printing page %u, %u%% complete...\n",page,percent);
    fprintf(stderr,"ATTR: job-media-progress=%u\n",percent);
}
static int terminated(void *unused) {(void)unused;return stop_requested!=0;}

static const char *ppd_option(ppd_file_t *ppd,const char *name,
                              const char *fallback) {
    ppd_choice_t *c=ppdFindMarkedChoice(ppd,name);
    return c && c->choice[0] ? c->choice : fallback;
}
int main(int argc,char **argv) {
    if(argc!=6 && argc!=7) return 1;
    const char *ppdpath=getenv("PPD");
    if(!ppdpath) return 1;
    ppd_file_t *ppd=ppdOpenFile(ppdpath);
    if(!ppd) return 1;
    cups_option_t *arguments=NULL;
    int n=cupsParseOptions(argv[5],0,&arguments);
    ppdMarkDefaults(ppd);
    for(int i=0;i<n;++i) ppdMarkOption(ppd,arguments[i].name,
                                            arguments[i].value);
    cupsMarkOptions(ppd,n,arguments);
    classic_raster_job_options cfg={0};
    cfg.high_resolution = !strcasecmp(ppd_option(ppd,"Resolution","300dpi"),
                                      "600dpi");
    int quality=classic_raster_quality_selector(
               ppd_option(ppd,"PrintQuality","Bayer8x8"));
    cfg.quality_selector=quality < 0 ? 0u : (unsigned)quality;
    cfg.optimization=(unsigned)atoi(ppd_option(ppd,"OptimizationLevel","2"));
    cfg.cut_every_page=!strcasecmp(ppd_option(ppd,"LabelCutter","OFF"),"ON");
    int deferred_prn=!strcasecmp(ppd_option(ppd,"PrintThroughPRN","OFF"),"ON");
    const char *uri=getenv("DEVICE_URI");
    int network=uri && strstr(uri,"ipp")!=NULL;
    cupsFreeOptions(n,arguments);
    ppdClose(ppd);
    if(network) {
        fputs("ERROR: classic macOS IPP output reconstruction is incomplete\n",
              stderr);
        return 1;
    }
    signal(SIGTERM,handle_term);
    signal(SIGPIPE,SIG_IGN);
    int fd=STDIN_FILENO,opened=0;
    if(argc==7) {
        fd=open(argv[6],O_RDONLY);
        if(fd<0) return 1;
        opened=1;
    }
    cups_raster_t *r=cupsRasterOpen(fd,CUPS_RASTER_READ);
    if(!r) {
        if(opened) close(fd);
        return 1;
    }
    input inp={0};
    inp.handle=r;
    classic_original_prn_state prn={0};
    if(deferred_prn) classic_original_prn_open(&prn);
    classic_raster_job_io io={
        {&inp,read_header,read_pixels},deferred_prn ? (void*)&prn : NULL,
        deferred_prn ? emit_prn : emit_stdout,
        deferred_prn ? flush_prn : flush_stdout,
        progress_stdout,terminated
    };
    unsigned pages=0;
    int status=classic_raster_job_run(&io,&cfg,&pages);
    cupsRasterClose(r);
    if(opened) close(fd);
    if(deferred_prn) {
        classic_original_prn_close(&prn);
        if(status==0) classic_original_prn_send(&prn,stdout);
    }
    return status ? 1 : 0;
}
