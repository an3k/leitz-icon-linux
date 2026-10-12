/* Independent CUPS raster filter for Leitz Icon.
 * Source-derived 86-profile media catalog; no PDF cropping or application logic.
 * Continuous roll uses job-dependent length; historical 170 mm was one test.
 */
#include "leitz_jobs.h"
#include "leitz_media.h"
#include <cups/raster.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static int output(FILE *in, FILE *out){
 unsigned char buf[32768];size_t n;
 while((n=fread(buf,1,sizeof(buf),in)))if(fwrite(buf,1,n,out)!=n)return -1;
 return ferror(in)?-1:0;
}
static int option(const char *s,const char *key,const char *value){
 size_t len=strlen(key);
 for(const char *p=s;p&&*p;){
  while(*p==' ')p++;
  if(!strncmp(p,key,len)&&p[len]=='=' &&
    !strncmp(p+len+1,value,strlen(value)) &&
    (p[len+1+strlen(value)]==' '||p[len+1+strlen(value)]=='\0'))return 1;
  while(*p&&*p!=' ')p++;
 }
 return 0;
}

/* Vendor PPD names are preserved. The macOS raster implementation is not
 * provided; non-Bayer quality functions below are explicit Current
 * approximations, not byte-identical implementations of macOS algorithms. */
static const char *const quality_names[]={
 "Draft","SuperCell","Bayer4x4","Bayer8x8","Bayer8x8SuperCell1",
 "Bayer8x8SuperCell2","CUPSThresold64x64","SuperCell1","SuperCell2",
 "SuperCell3","Bayer14x14","Clustered8x8","Dispersed8x8",
 "Nasik4x4","Dithering3x3"
};
static unsigned bayer_value(unsigned x,unsigned y,unsigned bits){
 unsigned value=0;
 for(unsigned i=0;i<bits;i++){
   unsigned xi=(x>>i)&1u, yi=(y>>i)&1u;
   unsigned part=yi?(xi?1u:3u):(xi?2u:0u);
   value+=part << (2u*(bits-1u-i));
 }
 return value;
}
static unsigned ordered_threshold(unsigned kind,unsigned x,unsigned y){
 unsigned v,den;
 switch(kind){
  case 0: return 127u; /* Draft: threshold, no synthetic gray pattern. */
  case 2: v=bayer_value(x,y,2);den=16u;break;
  case 3: case 12: v=bayer_value(x,y,3);den=64u;break;
  case 4: v=(bayer_value(x,y,3)+((x+y)&1u)*16u)%64u;den=64u;break;
  case 5: v=(bayer_value(x,y,3)+((x^y)&1u)*32u)%64u;den=64u;break;
  case 6: v=bayer_value(x,y,6);den=4096u;break;
  case 10: v=(bayer_value(x,y,4)*196u)/256u;den=196u;break;
  case 14: v=(x%3u*3u+y%3u*5u)%9u;den=9u;break;
  case 13: v=(bayer_value(x,y,2)+((x+y)%3u)*3u)%16u;den=16u;break;
  default: {
    /* Clustered and SuperCell modes use dot-center ranking. The original
     * Apple/Leitz kernels are not available; sizes and variants remain
     * separate adjustable strategies. */
    const unsigned n=(kind==1||kind==11)?8u:(kind==7?4u:(kind==8?6u:8u));
    unsigned a=x%n,b=y%n, cx=n/2u,cy=n/2u;
    unsigned dx=a>cx?a-cx:cx-a,dy=b>cy?b-cy:cy-b;
    v=dx*dx+dy*dy;
    if(kind==9)v=(v*3u+a+b)% (n*n);
    else v=(v*n+a+(b*3u))%(n*n);
    den=n*n;
    break;
  }
 }
 return (2u*v+1u)*255u/(2u*den);
}
static int quality_mode(const char *options){
 int chosen=3; /* Original macOS default Bayer8x8. */
 for(unsigned j=0;j<4;j++){
  static const char *const ht[]={
   "HT_PATSIZE_AUTO","HT_PATSIZE_SUPERCELL_M",
   "HT_PATSIZE_6x6_M","HT_PATSIZE_8x8_M"
  };
  if(option(options,"Halftone",ht[j]))
    chosen=(int)(j==0?3:j==1?1:j==2?8:3);
 }
 if(strstr(options,"Halftone=")){
  int valid=0;
  for(unsigned j=0;j<4;j++){
   static const char *const ht[]={
    "HT_PATSIZE_AUTO","HT_PATSIZE_SUPERCELL_M",
    "HT_PATSIZE_6x6_M","HT_PATSIZE_8x8_M"
   };
   valid|=option(options,"Halftone",ht[j]);
  }
  if(!valid)return -1;
 }
 int quality_specified=0;
 for(unsigned j=0;j<sizeof(quality_names)/sizeof(quality_names[0]);j++){
  if(option(options,"PrintQuality",quality_names[j])){
   chosen=(int)j;quality_specified=1;break;
  }
 }
 if(strstr(options,"PrintQuality=")&&!quality_specified)return -1;
 if(quality_specified)return chosen;
 if(strstr(options,"Halftone="))return chosen;
 if(option(options,"LeitzDither","Bayer4"))return 2;
 if(option(options,"LeitzDither","Threshold"))return 0;
 if(strstr(options,"LeitzDither="))return -1;
 return chosen;
}

/* A custom CUPS page can happen to have the same dimensions as a
 * vendor die-cut profile. Geometry alone must not turn that custom job
 * into a fixed-media job with a vendor length command. */
static int explicitly_custom_page(const char *options){
 const char *keys[]={"PageSize=Custom","media=Custom"};
 for(const char *p=options;p&&*p;){
   while(*p==' ')p++;
   for(size_t i=0;i<sizeof(keys)/sizeof(keys[0]);i++){
     const size_t len=strlen(keys[i]);
     if(!strncmp(p,keys[i],len) &&
        (p[len]==' ' || p[len]=='\0' || p[len]=='.'))return 1;
   }
   while(*p&&*p!=' ')p++;
 }
 return 0;
}
static const struct leitz_media *lookup(const cups_page_header2_t *h){
 for(size_t i=0;i<LEITZ_MEDIA_COUNT;i++){
  const struct leitz_media *m=&leitz_media_catalog[i];
  double pw=m->page_width*72.0/300.0, ph=m->page_height*72.0/600.0;
  if(fabs(h->cupsPageSize[0]-pw)<.4 && fabs(h->cupsPageSize[1]-ph)<.4 &&
     h->cupsWidth==m->raster_width &&
     h->cupsHeight==(h->HWResolution[1]==300?(m->raster_height+1)/2:m->raster_height))
     return m;
 }
 return NULL;
}
static int process(cups_raster_t *r,const cups_page_header2_t *h,leitz_job *job,const char *options){
 unsigned dpi=h->HWResolution[1];
 unsigned width=h->cupsWidth,height=h->cupsHeight;
 const int grayscale8=(h->cupsBitsPerPixel==8 && h->cupsBitsPerColor==8);
 const unsigned useful_bytes=grayscale8?width:(width+7)/8;
 if(h->HWResolution[0]!=300 || (dpi!=300&&dpi!=600) || !width||width>LEITZ_MAX_WIDTH ||
    !height || height>LEITZ_MAX_ROWS ||
    !((h->cupsBitsPerPixel==1 && h->cupsBitsPerColor==1) || grayscale8) ||
    h->cupsColorOrder!=CUPS_ORDER_CHUNKED ||
    (h->cupsColorSpace!=CUPS_CSPACE_K&&h->cupsColorSpace!=CUPS_CSPACE_W) ||
    h->cupsBytesPerLine<useful_bytes ||
    h->cupsBytesPerLine>useful_bytes+63){
    /* CUPS raster can pad the physical row stride. The first ceil(width/8)
     * bytes contain the meaningful pixels; trailing stride bytes are padding.
     * Limit accepted padding to one 64-byte alignment unit. */
     fprintf(stderr,"ERROR: Unsupported raster: %u x %u @ %u x %u, depth=%u, bpl=%u\n",
      width,height,h->HWResolution[0],dpi,h->cupsBitsPerPixel,h->cupsBytesPerLine);
     return -1;
 }
 int custom_page=explicitly_custom_page(options);
 const int debug_mesh=option(options,"DebugMode","Mesh");
 const int debug_log=option(options,"DebugMode","Debug");
 const int quality=quality_mode(options);
 const int forced_continuous=option(options,"ContinuousMedia","ON");
 const int correct_margins=option(options,"ShouldCorrectMargins","YES");
 if(quality<0){fprintf(stderr,"ERROR: Unsupported PrintQuality/Halftone/Dither option\n");return -1;}
 const struct leitz_media *m=custom_page?NULL:lookup(h);
 /* If the print job explicitly selects a vendor media name, ensure that
  * its raster geometry matches. Do not reinterpret malformed vendor pages
  * as a custom size and transmit them. */
 for(size_t i=0;i<LEITZ_MEDIA_COUNT;i++){
   if(!custom_page &&
      (option(options,"PageSize",leitz_media_catalog[i].name) ||
       option(options,"media",leitz_media_catalog[i].name))){
      const struct leitz_media *selected=&leitz_media_catalog[i];
      if(selected->page_width*72.0/300.0 < h->cupsPageSize[0]-.4 ||
         selected->page_width*72.0/300.0 > h->cupsPageSize[0]+.4 ||
         selected->page_height*72.0/600.0 < h->cupsPageSize[1]-.4 ||
         selected->page_height*72.0/600.0 > h->cupsPageSize[1]+.4 ||
         width!=selected->raster_width ||
         height!=(dpi==300?(selected->raster_height+1)/2:selected->raster_height)){
        fprintf(stderr,"ERROR: PageSize %s does not match CUPS raster geometry\n",selected->name);
        return -1;
      }
      /* Preserve the explicitly selected vendor profile, including
       * its source-specific length code and geometry. Some A/B entries
       * deliberately share dimensions but not necessarily semantics. */
      m=selected;
      break;
   }
 }
 int fixed=m&&!m->continuous;
 int continuous=m&&m->continuous;
 if(forced_continuous && fixed){
   fprintf(stderr,"ERROR: ContinuousMedia ON conflicts with a fixed label profile\n");
   return -1;
 }
 /* The original option describes the *installed* cartridge. Its physical
  * identity is not available in this CUPS raster filter; never silently
  * reinterpret die-cut stock as a continuous roll. The USB backend
  * verifies the actual cartridge separately. */
 if(debug_log)
  fprintf(stderr,"DEBUG: media=%s pagesize=%gx%g raster=%ux%u dpi=300x%u quality=%s continuous=%d\n",
    m?m->name:"Custom",h->cupsPageSize[0],h->cupsPageSize[1],
    width,height,dpi,quality_names[quality],continuous||forced_continuous);
 int page_mode=option(options,"LeitzCutMode","Page");
 int force_content=option(options,"LeitzCutMode","Content");
 /* Content-length cutting is supported only for roll/custom media.
  * Using it on a pre-cut label changes the physical label length and
  * can cut through the installed stock. Do not silently override the
  * fixed manufacturer's length command. */
 if(force_content && fixed){
   fprintf(stderr,"ERROR: Content cut mode is not valid for fixed media %s; select Automatic or Page\n",m->name);
   return -1;
 }
 /* Reject malformed sizes before conversion: lround(NaN/Inf/overflow)
  * is not a valid physical label length. */
 double measured_rows=h->cupsPageSize[1]*(double)dpi/72.0;
 if(!isfinite(measured_rows) || measured_rows<0.5 ||
    measured_rows>(double)LEITZ_MAX_ROWS+0.49){
   fprintf(stderr,"ERROR: Invalid CUPS page length\n");
   return -1;
 }
 unsigned original_total=(unsigned)lround(measured_rows);
 if(original_total==0||original_total>LEITZ_MAX_ROWS)return -1;
 unsigned head=0,tail=0,device_length=65535;
 if(force_content){
   /* Explicit roll-material override, regardless of chosen page size. */
   fixed=0;device_length=65535;
   head=(dpi==600)?182:91;tail=(dpi==600)?181:91;
 }else if(fixed){
   /* Preserve exact Windows GPD command, including its anomalous cards. */
   device_length=m->length_code;
   head=m->bottom*(dpi/300)/2; /* GPD y units at 600dpi -> physical dots */
   if(original_total<head+height){fprintf(stderr,"ERROR: Selected media height too short\n");return -1;}
   tail=original_total-head-height;
 }else if(continuous&&!page_mode){
   head=(dpi==600)?182:91;
   tail=(dpi==600)?181:91;
 }else if(!m&&!page_mode){
   /* CUPS custom pages on roll media, automatic by default. */
   head=(dpi==600)?182:91;tail=(dpi==600)?181:91;
 }else{
   head=(original_total>height)?(original_total-height)/2:0;
   if(original_total<head+height)return -1;
   tail=original_total-head-height;
 }
 if(page_mode && continuous && !force_content){
    fprintf(stderr,"ERROR: Full length of continuous GPD sheet is 2.7m. Select Automatic or a custom smaller page size.\n");
    return -1;
 }
 uint8_t *row=malloc(h->cupsBytesPerLine);
 uint8_t *bits=malloc(width);
 FILE *cached=tmpfile();
 if(!row||!bits||!cached){fprintf(stderr,"ERROR: Insufficient raster buffering resources\n");free(row);free(bits);if(cached)fclose(cached);return -1;}
 int rc=-1;unsigned last_ink=0;int has_ink=0;
 for(unsigned y=0;y<height;y++){
  if(cupsRasterReadPixels(r,row,h->cupsBytesPerLine)!=h->cupsBytesPerLine){fprintf(stderr,"ERROR: Incomplete input raster\n");goto done;}
  int present=0;
  for(unsigned x=0;x<width;x++){
    if(grayscale8){
      /* CUPS DeviceK is ink coverage, while DeviceGray/W is luminance.
       * Both paths must produce identical monochrome pixels from the same
       * photograph. Apply dithering before either protocol encoder. */
      unsigned ink=h->cupsColorSpace==CUPS_CSPACE_K?row[x]:255u-row[x];
      unsigned threshold=ordered_threshold((unsigned)quality,x,y);
      bits[x]=(uint8_t)(ink>threshold);
    }else{
      unsigned b=(row[x/8]>>(7-(x&7)))&1;
      bits[x]=(uint8_t)((h->cupsColorSpace==CUPS_CSPACE_K)?b:!b);
    }
    present|=bits[x];
  }
  /* Original Linux continuous margin correction discards the leading
   * 30/60 raster rows. Preserve their physical advance as white rows. */
  if(forced_continuous && correct_margins && y<=((dpi==600)?60u:30u)){
    memset(bits,0,width);present=0;
  }
  /* Vendor DebugMode=Mesh, adapted after color-space normalization. */
  if(debug_mesh){
    if(y==0 || y==height/2 || y==height-1){
      memset(bits,1,width);
    }else{
      bits[0]=1;
      bits[width/2]=1;
      bits[width-1]=1;
    }
    present=1;
  }
  if(fwrite(bits,1,width,cached)!=width){fprintf(stderr,"ERROR: Spool write failed\n");goto done;}
  if(present){last_ink=y+1;has_ink=1;}
 }
 if(!has_ink){fprintf(stderr,"ERROR: Empty label; no material consumed\n");goto done;}
 unsigned used= (fixed || page_mode)?height:last_ink;
 unsigned total=head+used+tail;
 if(total>LEITZ_MAX_ROWS){fprintf(stderr,"ERROR: Label length exceeds allowed maximum\n");goto done;}
 /* For 300x600, ESC L is in 300dpi units according to the Windows GPD.
    For custom size use 0xffff, as with the vendor's CUSTOMSIZE mode. */
 if(fseek(cached,0,SEEK_SET)){goto done;}
 /* Shared page controller implements feed, row and cut policy for both
  * encoders; a protocol choice must not affect media and raster processing. */
 if(leitz_job_begin_page(job,width,total,used,head,tail,device_length,dpi))goto done;
 for(unsigned y=0;y<used;y++){
   if(fread(bits,1,width,cached)!=width || leitz_job_row(job,bits))goto done;
 }
 if(leitz_job_end_page(job))goto done;
 fprintf(stderr,"INFO: Leitz protocol=%s media=%s width=%u dpi=300x%u feed=%u; content=%u, total=%u; cut=%s\n",
    leitz_job_protocol_name(job),m?m->name:"Custom",width,dpi,head,used,total,
    (fixed||page_mode)?"page":"after-content");
 rc=0;
done:
 fclose(cached);free(row);free(bits);return rc;
}
int main(int argc,char **argv){
 if(argc!=6&&argc!=7){fprintf(stderr,"ERROR: Invalid CUPS filter arguments\n");return 1;}
 int fd=argc==7?open(argv[6],O_RDONLY):STDIN_FILENO;
 if(fd<0){perror("Input");return 1;}
 cups_raster_t *r=cupsRasterOpen(fd,CUPS_RASTER_READ);
 if(!r){fprintf(stderr,"ERROR: Unable to open CUPS raster\n");if(argc==7)close(fd);return 1;}
 FILE *job=tmpfile();if(!job){perror("tmpfile");cupsRasterClose(r);if(argc==7)close(fd);return 1;}
 const leitz_protocol protocol=option(argv[5],"LeitzProtocol","Current")
   ? LEITZ_PROTOCOL_CURRENT : option(argv[5],"LeitzProtocol","Native")
   ? LEITZ_PROTOCOL_NATIVE : LEITZ_PROTOCOL_AUTO;
 if(strstr(argv[5],"LeitzProtocol=") &&
    !option(argv[5],"LeitzProtocol","Current") &&
    !option(argv[5],"LeitzProtocol","Native") &&
    !option(argv[5],"LeitzProtocol","Auto")){
  fprintf(stderr,"ERROR: Unknown LeitzProtocol selection\n");
  fclose(job);cupsRasterClose(r);if(argc==7)close(fd);return 1;
 }
 const int cut_each_page=option(argv[5],"CutMedia","True")||
   !option(argv[5],"LabelCutter","OFF");
 unsigned current_mode=option(argv[5],"LeitzCurrentRaster","FullWidthRaw")?1u:
   option(argv[5],"LeitzCurrentRaster","TrimmedRaw")?2u:
   option(argv[5],"LeitzCurrentRaster","NoSkipRaw")?3u:
   option(argv[5],"LeitzCurrentRaster","Optimized")?0u:
   option(argv[5],"OptimizationLevel","0")?3u:
   option(argv[5],"OptimizationLevel","1")?1u:
   option(argv[5],"OptimizationLevel","2")?2u:
   option(argv[5],"OptimizationLevel","3")?1u:2u;
 if((strstr(argv[5],"LeitzCurrentRaster=") &&
      !option(argv[5],"LeitzCurrentRaster","Optimized") &&
      !option(argv[5],"LeitzCurrentRaster","FullWidthRaw") &&
      !option(argv[5],"LeitzCurrentRaster","TrimmedRaw") &&
      !option(argv[5],"LeitzCurrentRaster","NoSkipRaw")) ||
    (strstr(argv[5],"OptimizationLevel=") &&
      !option(argv[5],"OptimizationLevel","0") &&
      !option(argv[5],"OptimizationLevel","1") &&
      !option(argv[5],"OptimizationLevel","2") &&
      !option(argv[5],"OptimizationLevel","3"))){
  fprintf(stderr,"ERROR: Unknown LeitzCurrentRaster option\n");
  fclose(job);cupsRasterClose(r);if(argc==7)close(fd);return 1;
 }
 leitz_job *document=leitz_job_create(job,protocol,cut_each_page);
 /* Original Linux optimization is a Current encoder control only.
  * Never pass a nonzero Current raster selector to the unchanged Native
  * protocol, even when the vendor OptimizationLevel default is 2. */
 if(document && strcmp(leitz_job_protocol_name(document),"Current"))
   current_mode=0;
 if(!document || leitz_job_set_current_raster_mode(document,current_mode)){
  fprintf(stderr,"ERROR: Current raw mode requires the Current protocol module\n");
  leitz_job_destroy(document);fclose(job);cupsRasterClose(r);
  if(argc==7)close(fd);
  return 1;
 }
 cups_page_header2_t h;int pages=0,failed=0;
 while(!failed && cupsRasterReadHeader2(r,&h)){
   pages++;
   if(pages>64 || process(r,&h,document,argv[5])){
     fprintf(stderr,"ERROR: Unsupported or invalid Leitz raster page\n");
     failed=1;break;
   }
 }
 cupsRasterClose(r);if(argc==7)close(fd);
 if(!pages){fprintf(stderr,"ERROR: No raster pages\n");failed=1;}
 if(!failed && leitz_job_finish(document)){
   fprintf(stderr,"ERROR: Cannot finish Leitz print job\n");
   failed=1;
 }
 if(!failed){
   char reason[256]={0};
   if(leitz_job_validate(document,reason,sizeof reason)){
     fprintf(stderr,"ERROR: Generated job failed validation: %s\n",reason);
     failed=1;
   }
 }
 leitz_job_destroy(document);
 if(!failed){rewind(job);if(output(job,stdout)||fflush(stdout)){fprintf(stderr,"ERROR: Output failure\n");failed=1;}}
 fclose(job);
 return failed?1:0;
}
