/* Independent wire-format reference for Esselte's 2014 Linux v1.0.3 driver.
 * Distinct from production leitz_codec.c. No USB access or CUPS dependencies.
 */
#include "leitz_protocol.h"
typedef struct {
  FILE *out;
  unsigned width, total_rows, rows_written, pending_blank, dpi_y, bottom_rows;
  int failed, finished;
} leitz_native_reference;
#include <string.h>

#define ESC 0x1bu
#define SYN 0x16u

static int send(FILE *f, const uint8_t *data, size_t len) {
  return fwrite(data, 1, len, f) == len ? 0 : -1;
}
static int skip_rows(leitz_native_reference *p, unsigned rows) {
  while (rows) {
    unsigned step = rows > 65535u ? 65535u : rows;
    const uint8_t command[4] = {ESC, 'f', (uint8_t)(step >> 8), (uint8_t)step};
    if (send(p->out, command, sizeof command)) return -1;
    rows -= step;
  }
  return 0;
}
int leitz_native_begin_page(leitz_native_reference *p, FILE *out,
                            unsigned width, unsigned rows, unsigned dpi_y,
                            unsigned length_units, int first_page) {
  if (!p || !out || !width || width > 960 || !rows || rows > 64000 ||
      (dpi_y != 300 && dpi_y != 600) || length_units > 65535)
    return -1;
  memset(p, 0, sizeof *p);
  p->out = out;
  p->width = width;
  p->total_rows = rows;
  p->dpi_y = dpi_y;
  const uint8_t init[4] = {ESC, '@', ESC, dpi_y == 600 ? 'i' : 'h'};
  uint8_t preamble[120];
  memset(preamble, ESC, sizeof preamble);
  const uint8_t setup[11] = {ESC, 'L', (uint8_t)(length_units >> 8),
    (uint8_t)length_units, ESC, 'w', (uint8_t)(width >> 8),
    (uint8_t)width, ESC, 'B', 0};
  if ((first_page && send(out, init, sizeof init)) ||
      send(out, preamble, sizeof preamble) ||
      send(out, setup, sizeof setup)) return p->failed = -1;
  return 0;
}
int leitz_native_begin(leitz_native_reference *p, FILE *out,
                       unsigned width, unsigned rows, unsigned dpi_y,
                       unsigned length_units) {
  return leitz_native_begin_page(p, out, width, rows, dpi_y, length_units, 1);
}
int leitz_native_row(leitz_native_reference *p, const uint8_t *packed,
                     unsigned row_bytes) {
  if (!p || p->failed || p->finished || !packed || p->rows_written >= p->total_rows ||
      row_bytes != (p->width + 7) / 8) return -1;
  p->rows_written++;
  unsigned ink = 0;
  for (unsigned i = 0; i < row_bytes; i++) ink |= packed[i];
  if (!ink) {
    p->pending_blank++;
    return 0;
  }
  if (skip_rows(p, p->pending_blank)) return p->failed = -1;
  p->pending_blank = 0;
  const uint8_t header[4] = {ESC, 'D', (uint8_t)row_bytes, SYN};
  if (send(p->out, header, sizeof header) ||
      send(p->out, packed, row_bytes)) return p->failed = -1;
  return 0;
}
int leitz_native_finish_page(leitz_native_reference *p, int cut) {
  if (!p || p->failed || p->finished || p->rows_written != p->total_rows) return -1;
  /* Original 1.0.3 optimization-1 leaves trailing blank rows pending.
   * Inter-label page end is ESC G, with optional ESC E cutter command.
   * The document always ends with ESC G ESC E. */
  const uint8_t finish[4] = {ESC, 'G', ESC, 'E'};
  if (send(p->out, finish, cut ? 4u : 2u)) return p->failed = -1;
  p->finished = 1;
  return 0;
}
int leitz_native_end(leitz_native_reference *p) {
  return leitz_native_finish_page(p, 1);
}
int leitz_native_finish_job(leitz_native_reference *p) {
  /* The original Linux job closes with ESC E after the last ESC G. */
  if(!p || p->failed || !p->finished || !p->out)return -1;
  const uint8_t command[]={ESC,'E'};
  return send(p->out,command,sizeof command);
}

/* Native document frame validator. This is not shared Current code. */
/* Strict manufacturer Linux 1.0.3 plain/raw document framing validator.
 * Multiple pages share one ESC @/resolution setup. ESC G ends a page,
 * ESC E optionally cuts between pages and is REQUIRED at document end.
 * Production safety gate before any USB open/write; no device I/O here.
 */

#include <stdio.h>
#include <stdint.h>

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

/* Independent native single-page validation reference. */
/* Strict single-label validator for the native Linux v1.0.3 plain/raw subset.
 * It is intentionally NOT installed as a CUPS backend and never touches USB.
 */

#include <stdio.h>
#include <string.h>

static int bad(char *msg, size_t cap, const char *reason) {
  if (msg && cap) snprintf(msg, cap, "%s", reason);
  return -1;
}
static int match(FILE *f, const uint8_t *bytes, unsigned n) {
  for (unsigned i = 0; i < n; i++) if (fgetc(f) != bytes[i]) return -1;
  return 0;
}
int leitz_native_validate(FILE *in, unsigned *width_out, unsigned *rows_out,
                          char *error, size_t error_size) {
  if (!in || !width_out || !rows_out) return bad(error, error_size, "Missing inputs");
  *width_out = *rows_out = 0;
  const uint8_t reset[] = {0x1b, '@', 0x1b};
  if (match(in, reset, sizeof reset)) return bad(error, error_size, "No native reset");
  int dpi_mode = fgetc(in);
  if (dpi_mode != 'h' && dpi_mode != 'i') return bad(error, error_size, "Bad native resolution");
  for (unsigned i = 0; i < 120; i++)
    if (fgetc(in) != 0x1b) return bad(error, error_size, "Bad native page preamble");
  const uint8_t len_cmd[] = {0x1b, 'L'};
  if (match(in, len_cmd, sizeof len_cmd)) return bad(error, error_size, "Missing label length");
  int length_hi = fgetc(in), length_lo = fgetc(in);
  if (length_hi < 0 || length_lo < 0) return bad(error, error_size, "Truncated label length");
  const uint8_t width_cmd[] = {0x1b, 'w'};
  if (match(in, width_cmd, sizeof width_cmd)) return bad(error, error_size, "Missing print width");
  int hi = fgetc(in), lo = fgetc(in);
  if (hi < 0 || lo < 0) return bad(error, error_size, "Truncated print width");
  unsigned width = (unsigned)hi * 256 + (unsigned)lo;
  if (!width || width > 960) return bad(error, error_size, "Bad print width");
  const uint8_t b0[] = {0x1b, 'B', 0};
  if (match(in, b0, sizeof b0)) return bad(error, error_size, "Missing native B0");
  unsigned rows = 0, printed = 0;
  for (;;) {
    int a = fgetc(in), b = fgetc(in);
    if (a != 0x1b || b < 0) return bad(error, error_size, "Unexpected or missing command");
    if (b == 'G') {
      const uint8_t end[] = {0x1b, 'E'};
      if (match(in, end, sizeof end) || fgetc(in) != EOF)
        return bad(error, error_size, "Invalid native footer");
      if (!printed) return bad(error, error_size, "Empty printed label");
      *width_out = width;
      *rows_out = rows;
      if (error && error_size) snprintf(error, error_size, "OK");
      return 0;
    }
    if (b == 'f') {
      int fh = fgetc(in), fl = fgetc(in);
      if (fh < 0 || fl < 0) return bad(error, error_size, "Truncated feed command");
      unsigned count = (unsigned)fh * 256 + (unsigned)fl;
      if (!count || count > 64000 - rows) return bad(error, error_size, "Bad feed length");
      rows += count;
      continue;
    }
    if (b != 'D') return bad(error, error_size, "Unsupported native row command");
    int n = fgetc(in);
    if (n != (int)((width + 7) / 8) || fgetc(in) != 0x16)
      return bad(error, error_size, "Bad native raster row header");
    if (rows >= 64000) return bad(error, error_size, "Too many raster rows");
    int ink = 0;
    for (int i = 0; i < n; i++) {
      int v = fgetc(in);
      if (v < 0) return bad(error, error_size, "Truncated raster row");
      ink |= v;
    }
    if (!ink) return bad(error, error_size, "Unoptimized blank row");
    rows++;
    printed++;
  }
}

/* Native Linux 1.0.3 adapter: preserve source-derived byte layout exactly.
 * Top and bottom white rows stay codec details; terminal blanks are NOT
 * flushed, matching the original reference even for continuous labels. */
static int native_begin(void *p,FILE *out,const leitz_page *m,int first_page){
 leitz_native_reference *n=p;
 static const uint8_t blank[120]={0};
 unsigned packed=(m->width+7u)/8u;
 if(leitz_native_begin_page(n,out,m->width,m->total_rows,m->dpi_y,
                            m->length_code,first_page))return -1;
 n->bottom_rows=m->bottom;
 for(unsigned i=0;i<m->top;i++)
  if(leitz_native_row(n,blank,packed))return -1;
 return 0;
}
static int native_row(void *p,const uint8_t *pixels,unsigned width){
 leitz_native_reference *n=p;
 uint8_t packed[120]={0};
 if(width!=n->width || width>LEITZ_MAX_WIDTH)return -1;
 for(unsigned i=0;i<width;i++)
  if(pixels[i])packed[i/8u]|=(uint8_t)(1u<<(7u-(i&7u)));
 return leitz_native_row(n,packed,(width+7u)/8u);
}
static int native_end(void *p,int cut){
 leitz_native_reference *n=p;
 const uint8_t blank[120]={0};
 unsigned packed=(n->width+7u)/8u;
 for(unsigned i=0;i<n->bottom_rows;i++)
  if(leitz_native_row(n,blank,packed))return -1;
 return leitz_native_finish_page(n,cut);
}
static int native_finish(void *p){return leitz_native_finish_job(p);}
const leitz_protocol_ops leitz_native_driver={
 .name="NativeLinux103",.state_size=sizeof(leitz_native_reference),
 .begin=native_begin,.row=native_row,.end_page=native_end,
 .finish=native_finish,.validate=leitz_native_validate_job
};
