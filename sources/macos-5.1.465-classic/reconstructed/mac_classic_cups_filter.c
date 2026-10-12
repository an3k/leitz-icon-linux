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
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

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
  FILE *out=ctx;
  return fwrite(data,1,length,out) == length ? 0 : -1;
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
  const char *debug=choice(ppd,"DebugMode","Release");
  if(strcmp(debug,"Release") && strcmp(debug,"OFF"))bad=1;
  if(truth(choice(ppd,"PrintThroughPRN","OFF")))bad=1;
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
  mac_classic_job_io io={&src,&ctx,stdout_write,stdout};
  unsigned pages=0;
  int result=mac_classic_job_run(&io,&cfg,&pages);
  if(fflush(stdout)!=0)result=-1;
  cupsRasterClose(r);
  if(own_fd)close(fd);
  if(result<0){fprintf(stderr,"ERROR: Classic macOS filter reconstruction failed after %u page(s)\n",pages);return 1;}
  fprintf(stderr,"INFO: %u page(s) encoded (not hardware verified)\n",pages);
  return 0;
}
