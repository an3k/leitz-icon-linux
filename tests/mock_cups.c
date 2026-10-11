/* Test-only CUPS raster API mock. Reads PGM P5 rows; NOT a CUPS wire decoder. */
#include "cups/raster.h"
#include "../src/leitz_media.h"
#include "../src/leitz_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
struct mock_cups_raster_s { FILE *fp; unsigned width,height,y;int seen,grayscale8;};
cups_raster_t *cupsRasterOpen(int fd,int mode){
 (void)mode;struct mock_cups_raster_s *r=calloc(1,sizeof(*r));if(!r)return NULL;
 r->fp=fdopen(dup(fd),"rb");if(!r->fp){free(r);return NULL;}return r;
}
unsigned cupsRasterReadHeader2(cups_raster_t *r,cups_page_header2_t *h){
 /* A synthetic test file may contain several concatenated P5 pages.
  * A new header becomes available only after all pixels of the prior page. */
 if(r->seen && r->y < r->height)return 0;
 unsigned width,height,max;
 if(fscanf(r->fp,"P5\n%u %u\n%u\n",&width,&height,&max)!=3||width<1||width>960||height<1||max!=255)return 0;
 const char *name=getenv("MOCK_MEDIA_NAME");if(!name)name="Continuous70030001";
 const struct leitz_media *m=NULL;
 for(size_t i=0;i<LEITZ_MEDIA_COUNT;i++)if(!strcmp(name,leitz_media_catalog[i].name)){m=&leitz_media_catalog[i];break;}
 if(!m&&strcmp(name,"Custom"))return 0;
 unsigned dpi=getenv("MOCK_DPI300")?300:600;
 memset(h,0,sizeof(*h));
 h->HWResolution[0]=300;h->HWResolution[1]=dpi;
 h->cupsWidth=getenv("MOCK_BAD_SIZE")?width-1:width;
 h->cupsHeight=height;
 h->cupsPageSize[0]=m?m->page_width*72.f/300.f:(width+79)*72.f/300.f;
 h->cupsPageSize[1]=m?m->page_height*72.f/600.f:(height+60)*72.f/dpi;
 r->grayscale8=getenv("MOCK_GRAY8")!=NULL;
 h->cupsBitsPerPixel=r->grayscale8?8:1;h->cupsBitsPerColor=h->cupsBitsPerPixel;
 h->cupsBytesPerLine=r->grayscale8?width:(width+7)/8;
 h->cupsColorOrder=CUPS_ORDER_CHUNKED;
 h->cupsColorSpace=getenv("MOCK_COLORSPACE_W")?CUPS_CSPACE_W:CUPS_CSPACE_K;
 r->seen++;r->y=0;r->width=width;r->height=height;return 1;
}
unsigned cupsRasterReadPixels(cups_raster_t *r,unsigned char *p,unsigned len){
 if(r->y>=r->height||len!=(r->grayscale8?r->width:(r->width+7)/8))return 0;
 unsigned char row[960];if(fread(row,1,r->width,r->fp)!=r->width)return 0;
 if(r->grayscale8){
   /* PGM is DeviceGray (0=black); DeviceK is ink coverage (255=black). */
   for(unsigned x=0;x<r->width;x++)
     p[x]=getenv("MOCK_COLORSPACE_W")?row[x]:(unsigned char)(255u-row[x]);
 }else{
   memset(p,0,len);
   for(unsigned x=0;x<r->width;x++)if(row[x]<128)p[x/8]|=(uint8_t)(1u<<(7-(x&7)));
   if(getenv("MOCK_COLORSPACE_W"))for(unsigned i=0;i<len;i++)p[i]=~p[i];
 }
 r->y++;return len;
}
void cupsRasterClose(cups_raster_t *r){if(r){fclose(r->fp);free(r);}}
