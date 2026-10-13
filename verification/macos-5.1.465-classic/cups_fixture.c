#include <cups/raster.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc,char **argv) {
 if(argc<2 || argc>4)return 2;
 int rgb=argc>=3 && !strcmp(argv[2],"rgb");
 unsigned height=argc==4 ? (unsigned)strtoul(argv[3],NULL,10) : 80u;
 if(height==0 || height>1024)return 2;
 int fd=open(argv[1],O_CREAT|O_TRUNC|O_WRONLY,0600);
 if(fd<0)return 3;
 cups_raster_t *r=cupsRasterOpen(fd,CUPS_RASTER_WRITE);
 if(!r){close(fd);return 4;}
 for(unsigned pg=0;pg<2;++pg){
  cups_page_header2_t h={0};
  h.HWResolution[0]=300;h.HWResolution[1]=300;
  h.PageSize[0]=249;h.PageSize[1]=19;
  h.cupsWidth=704;h.cupsHeight=height;
  h.cupsBytesPerLine=rgb ? 2112 : 88;
  h.cupsBitsPerColor=rgb ? 8 : 1;
  h.cupsBitsPerPixel=rgb ? 24 : 1;
  h.cupsColorSpace=rgb ? CUPS_CSPACE_RGB : CUPS_CSPACE_K;
  h.cupsColorOrder=CUPS_ORDER_CHUNKED;h.NumCopies=1;
  if(!cupsRasterWriteHeader2(r,&h))return 5;
  for(unsigned y=0;y<h.cupsHeight;++y){
   uint8_t row[2112]={0};
   if(rgb){
    for(unsigned x=0;x<704;++x){
      unsigned seed=(x*7u+y*13u+pg*23u)%256u;
      row[x*3u]=(uint8_t)seed;
      row[x*3u+1u]=(uint8_t)((seed+47u)%256u);
      row[x*3u+2u]=(uint8_t)((seed+113u)%256u);
    }
    if(cupsRasterWritePixels(r,row,2112)!=2112)return 6;
    continue;
   }
   if(y%7==1)memset(row,0xff,sizeof row);
   if(y%7==2){row[1]=0xff;row[2]=0xff;}
   if(y%7==3)row[0]=0xff;
   if(y%7==4)for(unsigned x=1;x<70;++x)row[x]=0x55;
   if(y%7==5)for(unsigned x=10;x<40;++x)row[x]=0xf0;
   if(y%7==6){row[22]=0x01;row[23]=0x80;}
   if(pg==1&&y==0)row[1]=0x80;
   if(cupsRasterWritePixels(r,row,88)!=88)return 6;
  }
 }
 cupsRasterClose(r);close(fd);
 puts("CUPS raster reference fixture generated");
 return 0;
}
