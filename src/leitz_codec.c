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
int leitz_end_page(leitz_encoder *e,int cut){
 if(e->error||e->emitted_rows!=e->raster_rows)return -1;
 if(feed(e,e->pending_blank)||feed(e,e->bottom))return -1;
 /* Cut policy belongs to the job coordinator. The wire encoder only
  * renders its chosen command sequence. The default full trailer is
  * byte-for-byte identical to the hardware-tested v0.4.1 behavior. */
 static const unsigned char end_cut[]={27,'G',27,'E',27,'@',27,'Z'};
 static const unsigned char end_feed[]={27,'G'};
 const unsigned char *end=cut?end_cut:end_feed;
 size_t end_size=cut?sizeof end_cut:sizeof end_feed;
 if(fwrite(end,1,end_size,e->out)!=end_size)return -1;
 return e->feed_units+e->printed_rows==e->total_rows?0:-1;
}
int leitz_end(leitz_encoder *e){return leitz_end_page(e,1);}
int leitz_finish_job(leitz_encoder *e){
  /* End-of-document cut after ESC G from a deliberately uncut final page.
   * The full trailer matches the existing Current encoding. */
  if(!e || e->error || !e->out || e->emitted_rows!=e->raster_rows)return -1;
  static const uint8_t trailer[]={27,'E',27,'@',27,'Z'};
  return fwrite(trailer,1,sizeof trailer,e->out)==sizeof trailer?0:-1;
}
static int rd(FILE *f){return fgetc(f);}
static int bad(char *r,size_t n,const char *m){if(n)snprintf(r,n,"%s",m);return -1;}
int leitz_validate(FILE *f,unsigned *pages,unsigned *max_width,char *r,size_t n){
 if(!f||!pages||!max_width)return bad(r,n,"Missing validation inputs");
 *pages=0;
 *max_width=0;
 unsigned pending_preamble=0;
 int last_page_cut=0;
 while(1){
  int v=pending_preamble?27:rd(f);
  if(v==EOF)break;
  if(v!=27)return bad(r,n,"Unexpected start byte");
  for(int i=1;i<120;i++)
    if((i<(int)pending_preamble?27:rd(f))!=27)
      return bad(r,n,"Invalid preamble");
  pending_preamble=0;
  const unsigned char a[]={27,'@',27,'B',0,27,'L'};
  for(size_t i=0;i<sizeof(a);i++)if(rd(f)!=a[i])return bad(r,n,"Header mismatch");
  int lh=rd(f),ll=rd(f);if(lh<0||ll<0)return bad(r,n,"Missing length code");
  int b=rd(f),c=rd(f),d=rd(f),e=rd(f);
  if(b!=27||(c!='i'&&c!='h')||d!=27||e!='w')return bad(r,n,"Missing resolution/width");
  int wh=rd(f),wl=rd(f);if(wh<0||wl<0)return bad(r,n,"Width truncated");
  unsigned width=(unsigned)(wh*256+wl);
  if(!width||width>LEITZ_MAX_WIDTH)return bad(r,n,"Raster width out of range");
  if(width>*max_width)*max_width=width;
  unsigned count=0;int linewidth=-1,finished=0;
  while(!finished){
   if(rd(f)!=27)return bad(r,n,"Body ESC missing");
   int cmd=rd(f);if(cmd<0)return bad(r,n,"Body truncated");
   if(cmd=='f'){
    int hi=rd(f),lo=rd(f);if(hi<0||lo<0)return bad(r,n,"Feed truncated");
    count+=(unsigned)(hi*256+lo);
   }else if(cmd=='D'){
    linewidth=rd(f);
    if(linewidth<1||linewidth>120)return bad(r,n,"Invalid raster width bytes");
   }else if(cmd=='B'){
    int x=rd(f),kind=rd(f);
    if(x<0||kind<0||linewidth<1||(unsigned)x*8+(unsigned)linewidth*8> ((width+7)/8)*8)
       return bad(r,n,"Invalid row offset/width");
    if(kind==0x16){for(int i=0;i<linewidth;i++)if(rd(f)<0)return bad(r,n,"Raw row truncated");}
    else if(kind==0x17){unsigned pixels=0;
      while(pixels<(unsigned)linewidth*8){int v2=rd(f);if(v2<0)return bad(r,n,"RLE truncated");pixels+=(unsigned)(v2&127)+1;}
      if(pixels!=(unsigned)linewidth*8)return bad(r,n,"RLE overrun");
    }else return bad(r,n,"Raster mode unknown");
    count++;
   }else if(cmd=='G'){
     if(rd(f)!=27)return bad(r,n,"Page boundary missing ESC");
     int next=rd(f);
     if(next=='E'){
       static const unsigned char rest[]={27,'@',27,'Z'};
       for(size_t i=0;i<sizeof(rest);i++)
         if(rd(f)!=rest[i])return bad(r,n,"Cut trailer missing");
       last_page_cut=1;
     }else if(next==27){
       /* The first two ESC bytes of the following 120-ESC preamble
        * have already been read. Do not mistake them for a cutter. */
       pending_preamble=2;
       last_page_cut=0;
     }else return bad(r,n,"Unknown page boundary");
     finished=1;
   }else return bad(r,n,"Unknown command");
   if(count>LEITZ_MAX_ROWS)return bad(r,n,"Print length exceeds safety maximum");
  }
  if(count==0)return bad(r,n,"Empty label");
  if((lh!=255||ll!=255) && count<(unsigned)(lh*256+ll))return bad(r,n,"Too few physical rows");
  (*pages)++;if(*pages>64)return bad(r,n,"Too many labels");
 }
 if(!*pages)return bad(r,n,"No labels");
 if(!last_page_cut)return bad(r,n,"Final page must include cut trailer");
 if(n)snprintf(r,n,"OK: %u validated label(s)",*pages);
 return 0;
}
