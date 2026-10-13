/* Reconstruction of the macOS 5.1.465 classic CUPS invocation path.
 * Binary: _raster2printer at 0x1000045a0..0x100006854:
 *  0x10000488d cupsRasterOpen; 0x1000048d9 ppdOpenFile;
 *  0x10000492d cupsParseOptions; 0x10000493f ppdMarkDefaults;
 *  0x100004ab2 ppdMarkOption; 0x100004c6f cupsMarkOptions;
 *  0x100004c88..0x100005288 ppdFindMarkedChoice;
 *  0x1000056d1 cupsRasterReadHeader2; 0x100005bfc cupsRasterReadPixels.
 * Only supported options are implemented. It does NOT silently emulate
 * unsupported vendor debug/network functions, and must not be used to print
 * until separately qualified. This adapter is not recovered vendor C.
 */
#include "mac_classic_job.h"
#include "mac_classic_dither.h"
#include "mac_classic_output.h"
#include "mac_classic_prn.h"
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* _main in classic Mach-O registers SIGTERM (15) and ignores SIGPIPE (13).
 * Do not signal mechanical completion or cut after cancellation. */
static volatile sig_atomic_t mac_classic_cancelled = 0;
static void mac_classic_sigterm(int sig) { (void)sig; mac_classic_cancelled=1; }
static int cups_cancelled(void *unused) { (void)unused; return mac_classic_cancelled!=0; }
typedef struct { cups_raster_t *raster; cups_page_header2_t h; } raster_context;

static int raster_header(void *v, mac_classic_page_info *info) {
  raster_context *c = v;
  if (!cupsRasterReadHeader2(c->raster,&c->h)) return 0;
  info->width=c->h.cupsWidth;
  info->height=c->h.cupsHeight;
  info->bytes_per_line=c->h.cupsBytesPerLine;
  info->bits_per_pixel=c->h.cupsBitsPerPixel;
  return 1;
}
static size_t raster_pixels(void *v,uint8_t *dst,size_t size) {
  raster_context *c=v;
  if (size > UINT_MAX) return 0;
  return cupsRasterReadPixels(c->raster,dst,(unsigned)size);
}
static int stdout_write(void *ctx,const uint8_t *data,size_t length) {
  mac_classic_output output={MAC_CLASSIC_STDOUT,
                             mac_classic_file_write,ctx,NULL,NULL};
  return mac_classic_write_data(&output,data,length);
}
/* Original _ProcessPage writes CUPS progress messages every 32 rows.
 * This is host-side encode progress, not mechanical device status. */
static void cups_progress(void *context, unsigned page, unsigned row, unsigned percent) {
  (void)context; (void)row;
  fprintf(stderr, "\nINFO: Printing page %u, %u%% complete...\n", page,percent);
  fprintf(stderr, "ATTR: job-media-progress=%u\n", percent);
}
static const char *choice(ppd_file_t *ppd,const char *key,const char *fallback) {
  ppd_choice_t *c=ppdFindMarkedChoice(ppd,key);
  return c && c->choice[0] ? c->choice : fallback;
}
static int truth(const char *s) {
  return !strcmp(s,"ON") || !strcmp(s,"True") || !strcmp(s,"true");
}
static int parse_optimization(const char *s,unsigned *value) {
  if (!s || !s[0] || s[1] || s[0]<'0' || s[0]>'3')return -1;
  *value=(unsigned)(s[0]-'0');return 0;
}
int main(int argc,char **argv) {
  if (signal(SIGTERM,mac_classic_sigterm)==SIG_ERR ||
      signal(SIGPIPE,SIG_IGN)==SIG_ERR) {
    fputs("ERROR: Cannot configure CUPS filter signal handlers\n",stderr);
    return 1;
  }
  if (argc!=6 && argc!=7) {
    fputs("ERROR: CUPS filter expects 5 or 6 arguments\n",stderr);
    return 1;
  }
  const char *ppd_path=getenv("PPD");
  if (!ppd_path || !*ppd_path) {
    fputs("ERROR: CUPS PPD path unavailable\n",stderr);return 1;
  }
  ppd_file_t *ppd=ppdOpenFile(ppd_path);
  if (!ppd) {fputs("ERROR: Unable to open original PPD\n",stderr);return 1;}
  cups_option_t *options=NULL;
  int count=cupsParseOptions(argv[5],0,&options);
  if(count<0){ppdClose(ppd);return 1;}
  ppdMarkDefaults(ppd);
  /* Matching original option marking; invalid options fail independently
   * only for those whose selected behavior is not yet reconstructed. */
  for (int i=0;i<count;++i) {
    if (options[i].name && options[i].value)
      ppdMarkOption(ppd,options[i].name,options[i].value);
  }
  cupsMarkOptions(ppd,count,options);
  mac_classic_job_config cfg={0};
  cfg.vertical_dpi=strcmp(choice(ppd,"Resolution","300dpi"),"600dpi")==0 ? 600u : 300u;
  int q=mac_classic_quality_from_ppd(choice(ppd,"PrintQuality","Bayer8x8"));
  int bad= q<0 || parse_optimization(choice(ppd,"OptimizationLevel","2"),&cfg.optimization_level);
  cfg.quality_selector= q<0 ? 0u : (unsigned)q;
  cfg.cut_each_label=truth(choice(ppd,"LabelCutter","OFF"));
  /* Classic macOS _raster2printer 0x1000050e3..0x1000051ab parses
   * Release, Debug and Mesh, but the resulting local mode is subsequently
   * used only by its informational options log (0x100005374). There is
   * no raster modification from this value in the examined classic binary.
   * In particular, do NOT transplant Linux 1.0.3's separate Mesh drawing. */
  const char *debug=choice(ppd,"DebugMode","Release");
  if(strcmp(debug,"Release") && strcmp(debug,"Debug") &&
     strcmp(debug,"Mesh")) bad=1;
  int through_prn=truth(choice(ppd,"PrintThroughPRN","OFF"));
  if(bad){
    fputs("ERROR: Unsupported or invalid classic macOS filter option\n",stderr);
    cupsFreeOptions(count,options);ppdClose(ppd);return 1;
  }
  cupsFreeOptions(count,options);
  ppdClose(ppd);
  int fd=STDIN_FILENO,own_fd=0;
  if(argc==7) {
    fd=open(argv[6],O_RDONLY);
    if(fd<0){perror("ERROR: CUPS raster input");return 1;}
    own_fd=1;
  }
  cups_raster_t *r=cupsRasterOpen(fd,CUPS_RASTER_READ);
  if (!r) {
    fputs("ERROR: Failed to open CUPS raster\n",stderr);
    if(own_fd)close(fd);
    return 1;
  }
  raster_context ctx={0};ctx.raster=r;
  mac_classic_page_source src={raster_header,raster_pixels};
  mac_classic_prn prn={0};
  if (through_prn && mac_classic_prn_open(&prn, stdout)) {
    fputs("ERROR: Cannot create deferred PRN spool\n",stderr);
    cupsRasterClose(r);
    if(own_fd)close(fd);
    return 1;
  }
  mac_classic_job_io io={&src,&ctx,
    through_prn ? mac_classic_prn_write : stdout_write,
    through_prn ? (void *)&prn : (void *)stdout,
    cups_progress,NULL,cups_cancelled,NULL};
  unsigned pages=0;
  int result=mac_classic_job_run(&io,&cfg,&pages);
  if(result == 0 && through_prn) result=mac_classic_prn_replay(&prn);
  mac_classic_prn_close(&prn);
  if(fflush(stdout)!=0)result=-1;
  cupsRasterClose(r);
  if(own_fd)close(fd);
  if(result<0){fprintf(stderr,"ERROR: Classic macOS filter reconstruction failed after %u page(s)\n",pages);return 1;}
  fprintf(stderr,"INFO: %u page(s) encoded (not hardware verified)\n",pages);
  return 0;
}
