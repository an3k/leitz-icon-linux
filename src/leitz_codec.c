/* Hardware verified: 960-pixel 300x600 raster protocol, 170 mm example.
 * Other widths/media/resolutions/lengths not yet physically verified.
 */
#include "leitz_codec.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static int put(FILE *f,unsigned v){return fputc(v&255,f)!=EOF;}
static int feed(leitz_encoder *e,unsigned n){
 while(n){unsigned part=n>65535?65535:n;
   if(!(put(e->out,27)&&put(e->out,'f')&&put(e->out,part>>8)&&put(e->out,part)))return -1;
   e->feed_units+=part;n-=part;
 }
 return 0;
}
int leitz_begin_media(leitz_encoder *e,FILE *out,unsigned width,unsigned total_rows,
 unsigned raster_rows,unsigned top,unsigned bottom,unsigned length_code,unsigned dpi_y){
 if(!out||width<1||width>LEITZ_MAX_WIDTH||total_rows>LEITZ_MAX_ROWS||
    top+bottom+raster_rows!=total_rows||!raster_rows||
    (dpi_y!=600&&dpi_y!=300)||length_code>65535)return -1;
 memset(e,0,sizeof(*e));e->out=out;e->width=width;
 e->raster_rows=raster_rows;e->total_rows=total_rows;
 e->top=top;e->bottom=bottom;e->previous_width=-1;
 for(int i=0;i<120;i++)if(!put(out,27))return -1;
 const unsigned char prefix[]={27,'@',27,'B',0,27,'L',(uint8_t)(length_code>>8),(uint8_t)length_code,
     27,dpi_y==600?'i':'h',27,'w',(uint8_t)(width>>8),(uint8_t)width};
 if(fwrite(prefix,1,sizeof(prefix),out)!=sizeof(prefix))return -1;
 if(feed(e,top))return -1;
 return 0;
}
int leitz_row_media(leitz_encoder *e,const uint8_t *pixels){
 if(e->error||e->emitted_rows>=e->raster_rows)return -1;
 e->emitted_rows++;
 int lo=(int)e->width,hi=-1;
 for(unsigned x=0;x<e->width;x++)if(pixels[x]){if((int)x<lo)lo=(int)x;hi=(int)x;}
 if(hi<0){e->pending_blank++;return 0;}
 if(e->pending_blank){if(feed(e,e->pending_blank))goto error;e->pending_blank=0;}
 unsigned left=(unsigned)lo/8,byte_end=(unsigned)hi/8+1,nbytes=byte_end-left;
 uint8_t packed[120]={0},runs[960];unsigned nr=0;
 for(unsigned x=0;x<nbytes*8;x++){
   unsigned pos=left*8+x;
   if(pos<e->width && pixels[pos])packed[x/8]|=(uint8_t)(1u<<(7-(x&7)));
 }
 for(unsigned x=0;x<nbytes*8;){
   unsigned v=(packed[x/8]>>(7-(x&7)))&1;
   unsigned len=1;
   while(x+len<nbytes*8 && len<128 && (((packed[(x+len)/8]>>(7-((x+len)&7)))&1)==v))len++;
   runs[nr++]=(uint8_t)((v?128:0)|(len-1));x+=len;
 }
 if(e->previous_width!=(int)nbytes){
   if(!(put(e->out,27)&&put(e->out,'D')&&put(e->out,nbytes)))goto error;
   e->previous_width=(int)nbytes;
 }
 int compressed=nr<nbytes;
 if(!(put(e->out,27)&&put(e->out,'B')&&put(e->out,left)&&put(e->out,compressed?0x17:0x16)))goto error;
 if(compressed){if(fwrite(runs,1,nr,e->out)!=nr)goto error;}
 else if(fwrite(packed,1,nbytes,e->out)!=nbytes)goto error;
 e->printed_rows++;
 return 0;
error:e->error=1;return -1;
}
int leitz_end(leitz_encoder *e){
 if(e->error||e->emitted_rows!=e->raster_rows)return -1;
 if(feed(e,e->pending_blank)||feed(e,e->bottom))return -1;
 static const unsigned char end[]={27,'G',27,'E',27,'@',27,'Z'};
 if(fwrite(end,1,sizeof(end),e->out)!=sizeof(end))return -1;
 return e->feed_units+e->printed_rows==e->total_rows?0:-1;
}
static int rd(FILE *f){return fgetc(f);}
static int bad(char *r,size_t n,const char *m){if(n)snprintf(r,n,"%s",m);return -1;}
int leitz_validate(FILE *f,unsigned *pages,char *r,size_t n){
 *pages=0;
 while(1){
  int v=rd(f);if(v==EOF)break;if(v!=27)return bad(r,n,"Unexpected start byte");
  for(int i=1;i<120;i++)if(rd(f)!=27)return bad(r,n,"Invalid preamble");
  const unsigned char a[]={27,'@',27,'B',0,27,'L'};
  for(size_t i=0;i<sizeof(a);i++)if(rd(f)!=a[i])return bad(r,n,"Header mismatch");
  int lh=rd(f),ll=rd(f);if(lh<0||ll<0)return bad(r,n,"Missing length code");
  int b=rd(f),c=rd(f),d=rd(f),e=rd(f);
  if(b!=27||(c!='i'&&c!='h')||d!=27||e!='w')return bad(r,n,"Missing resolution/width");
  int wh=rd(f),wl=rd(f);if(wh<0||wl<0)return bad(r,n,"Width truncated");
  unsigned width=(unsigned)(wh*256+wl);
  if(!width||width>LEITZ_MAX_WIDTH)return bad(r,n,"Raster width out of range");
  unsigned count=0,ink_rows=0;int linewidth=-1,finished=0;
  while(!finished){
   if(rd(f)!=27)return bad(r,n,"Body ESC missing");
   int cmd=rd(f);if(cmd<0)return bad(r,n,"Body truncated");
   if(cmd=='f'){
    int hi=rd(f),lo=rd(f);if(hi<0||lo<0)return bad(r,n,"Feed truncated");
    unsigned feed_rows=(unsigned)(hi*256+lo);
    if(!feed_rows)return bad(r,n,"Zero-length feed");
    count+=feed_rows;
   }else if(cmd=='D'){
    linewidth=rd(f);
    if(linewidth<1||linewidth>120)return bad(r,n,"Invalid raster width bytes");
   }else if(cmd=='B'){
    int x=rd(f),kind=rd(f);
    if(x<0||kind<0||linewidth<1||(unsigned)x*8+(unsigned)linewidth*8> ((width+7)/8)*8)
       return bad(r,n,"Invalid row offset/width");
    int ink=0;
    if(kind==0x16){
      for(int i=0;i<linewidth;i++){
        int v2=rd(f);if(v2<0)return bad(r,n,"Raw row truncated");
        if((width&7u) && ((unsigned)(x+i)+1u)*8u>width &&
           (v2 & ((1u<<(8u-(width&7u)))-1u)))
          return bad(r,n,"Ink in raster padding");
        ink|=v2;
      }
    }else if(kind==0x17){
      unsigned pixels=0;
      while(pixels<(unsigned)linewidth*8){
        int v2=rd(f);if(v2<0)return bad(r,n,"RLE truncated");
        unsigned len=(unsigned)(v2&127)+1u;
        if((v2&128) && (unsigned)x*8u+pixels+len>width)
          return bad(r,n,"Ink in raster padding");
        ink|=v2&128;
        pixels+=len;
      }
      if(pixels!=(unsigned)linewidth*8)return bad(r,n,"RLE overrun");
    }else return bad(r,n,"Raster mode unknown");
    if(!ink)return bad(r,n,"Blank raster row");
    ink_rows++;count++;
   }else if(cmd=='G'){
     static const unsigned char end[]={27,'E',27,'@',27,'Z'};
     for(size_t i=0;i<sizeof(end);i++)if(rd(f)!=end[i])return bad(r,n,"Footer missing");
     finished=1;
   }else return bad(r,n,"Unknown command");
   if(count>LEITZ_MAX_ROWS)return bad(r,n,"Print length exceeds safety maximum");
  }
  if(count==0||!ink_rows)return bad(r,n,"Empty label");
  if((lh!=255||ll!=255) && count<(unsigned)(lh*256+ll))return bad(r,n,"Too few physical rows");
  (*pages)++;if(*pages>64)return bad(r,n,"Too many labels");
 }
 if(ferror(f))return bad(r,n,"Spool read error");
 if(!*pages)return bad(r,n,"No labels");
 if(n)snprintf(r,n,"OK: %u validated label(s)",*pages);
 return 0;
}
