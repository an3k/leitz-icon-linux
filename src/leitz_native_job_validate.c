/* Strict manufacturer Linux 1.0.3 plain/raw document framing validator.
 * Multiple pages share one ESC @/resolution setup. ESC G ends a page,
 * ESC E optionally cuts between pages and is REQUIRED at document end.
 * Production safety gate before any USB open/write; no device I/O here.
 */
#include "leitz_native_reference.h"
#include <stdio.h>
#include <stdint.h>

#define ESC 0x1b
#define SYN 0x16
static int fail(char *e,size_t cap,const char *msg) {
  if(e && cap)snprintf(e,cap,"%s",msg);
  return -1;
}
static int take(FILE *f,int expected) {return fgetc(f)==expected?0:-1;}

int leitz_native_validate_job(FILE *in, unsigned *pages_out,
                              unsigned *max_width, char *error,
                              size_t error_size) {
  if (!in || !pages_out || !max_width)
    return fail(error,error_size,"Missing native document inputs");
  *pages_out=0;
  *max_width=0;
  if(take(in,ESC)||take(in,'@')||take(in,ESC))
    return fail(error,error_size,"Native job reset missing");
  int dpi=fgetc(in);
  if(dpi!='h' && dpi!='i')return fail(error,error_size,"Unknown native resolution");
  int pre0=-1,pre1=-1; /* 1 or 2 prefetched bytes after previous ESC G */
  for(unsigned page=0;page<64;page++) {
    for(unsigned i=0;i<120;i++) {
      int v=i==0 && pre0>=0?pre0:i==1 && pre1>=0?pre1:fgetc(in);
      if(v!=ESC)return fail(error,error_size,"Invalid native page preamble");
    }
    pre0=pre1=-1;
    if(take(in,ESC)||take(in,'L'))
      return fail(error,error_size,"Missing native page length");
    int hi=fgetc(in),lo=fgetc(in);
    if(hi<0||lo<0)return fail(error,error_size,"Truncated native page length");
    if(take(in,ESC)||take(in,'w'))
      return fail(error,error_size,"Missing native page width");
    hi=fgetc(in);lo=fgetc(in);
    if(hi<0||lo<0)return fail(error,error_size,"Truncated native width");
    unsigned width=(unsigned)hi*256u+(unsigned)lo;
    if(width==0||width>960)return fail(error,error_size,"Native width out of range");
    /* One physical cassette and one print width for the complete job.
     * Enforce the common geometry policy at the USB spool safety gate. */
    if(page && width!=*max_width)
      return fail(error,error_size,"Mixed raster widths in one job");
    if(!page)*max_width=width;
    if(take(in,ESC)||take(in,'B')||take(in,0))
      return fail(error,error_size,"Missing native page B0");
    unsigned rows=0,ink_rows=0;
    for (;;) {
      if(take(in,ESC))return fail(error,error_size,"Native page command missing");
      int cmd=fgetc(in);
      if(cmd=='G')break;
      if(cmd=='f'){
        int high=fgetc(in),low=fgetc(in);
        if(high<0||low<0)return fail(error,error_size,"Truncated native feed");
        unsigned count=(unsigned)high*256u+(unsigned)low;
        if(!count||count>64000u-rows)
          return fail(error,error_size,"Native feed length invalid");
        rows+=count;
      }else if(cmd=='D'){
        int n=fgetc(in);
        if(n!=(int)((width+7)/8)||take(in,SYN))
          return fail(error,error_size,"Native raster width/SYN mismatch");
        int any=0;
        for(int i=0;i<n;i++){
          int b=fgetc(in);
          if(b<0)return fail(error,error_size,"Truncated native raster");
          if(i==n-1 && (width&7u) &&
             (b & ((1u<<(8u-(width&7u)))-1u)))
            return fail(error,error_size,"Ink outside native raster width");
          any|=b;
        }
        if(!any||rows>=64000)return fail(error,error_size,"Invalid native ink row");
        rows++;ink_rows++;
      }else return fail(error,error_size,"Unknown native page command");
    }
    if(!ink_rows)return fail(error,error_size,"Empty native page");
    *pages_out=page+1;
    int after_g=fgetc(in);
    if(after_g!=ESC)return fail(error,error_size,"Native document final cut missing");
    int next=fgetc(in);
    if(next=='E') {
      int following=fgetc(in);
      if(following==EOF) {
        if(ferror(in))return fail(error,error_size,"Native input read error");
        if(error&&error_size)snprintf(error,error_size,"OK: %u native page(s)",*pages_out);
        return 0;
      }
      pre0=following;
    } else if(next==ESC) {
      pre0=ESC;pre1=ESC;  /* An inter-page ESC G without a cut */
    } else return fail(error,error_size,"Native document or page boundary invalid");
  }
  return fail(error,error_size,"Too many native pages");
}
