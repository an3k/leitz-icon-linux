#include "mac_classic_job.h"
#include "mac_classic_progress.h"
#include "mac_classic_geometry.h"
#include "mac_classic_process_page.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
 mac_classic_page_info pages[3];
 uint8_t data[3][128];
 unsigned count, index;
 unsigned truncated;
} test_source;
typedef struct { unsigned n, page[16], row[16], percent[16]; } test_progress;
typedef struct { unsigned checks, reject_on; } test_cancel;
static int cancel_after_n_checks(void *v) {
  test_cancel *c=v;
  return ++c->checks >= c->reject_on;
}
static void progress_seen(void *context,unsigned page,unsigned row,unsigned percent) {
  test_progress *p=context;
  assert(p->n<16);
  p->page[p->n]=page;p->row[p->n]=row;p->percent[p->n]=percent;
  ++p->n;
}
typedef struct {
 uint8_t data[2048];
 size_t n;
 size_t reject_after;
 test_cancel *cancel_at_final_row;
 uint8_t final_row_value;
} test_sink;

static int get_header(void *context,mac_classic_page_info *p) {
 test_source *s=context;
 if(s->index>=s->count) return 0;
 *p=s->pages[s->index];
 return 1;
}
static size_t get_pixels(void *context,uint8_t *b,size_t n) {
 test_source *s=context;
 if(s->index>=s->count||n>sizeof s->data[0]) return 0;
 if(s->truncated && n>1) --n;
 memcpy(b,s->data[s->index++],n);
 return n;
}
static int put_bytes(void *context,const uint8_t *b,size_t n) {
 test_sink *s=context;
 if(s->n+n>sizeof s->data || s->n+n>s->reject_after)return -1;
 memcpy(s->data+s->n,b,n);s->n+=n;
 if(s->cancel_at_final_row && n==1 && b[0]==s->final_row_value) {
   /* SIGTERM delivered during the last pixel write, not before it. */
   s->cancel_at_final_row->reject_on=s->cancel_at_final_row->checks+1;
 }
 return 0;
}
static int has(const test_sink *s,const uint8_t *needle,size_t n) {
 for(size_t i=0;i+n<=s->n;i++)if(memcmp(s->data+i,needle,n)==0)return 1;
 return 0;
}
static unsigned count_pattern(const test_sink *s,const uint8_t *needle,size_t n) {
 unsigned count=0;
 for(size_t i=0;i+n<=s->n;i++)if(memcmp(s->data+i,needle,n)==0)++count;
 return count;
}
static void init_sink(test_sink *s){memset(s,0,sizeof *s);s->reject_after=sizeof s->data;}
int main(void){
 unsigned value=99;
 assert(mac_classic_raster_progress(0,33,&value)==1 && value==0);
 assert(mac_classic_raster_progress(1,33,&value)==0 && value==0);
 assert(mac_classic_raster_progress(32,33,&value)==1 && value==96);
 assert(mac_classic_raster_progress(33,33,&value)==-1);
 assert(mac_classic_raster_progress(0,0,&value)==-1);
 assert(mac_classic_raster_progress(0,33,NULL)==-1);
 const mac_classic_page_source source={get_header,get_pixels};
 mac_classic_job_config cfg={.optimization_level=0,.quality_selector=0,.vertical_dpi=300,.cut_each_label=0};
 test_source ps={0};test_sink os;init_sink(&os);
 test_progress progress={0};
 mac_classic_job_io io={&source,&ps,put_bytes,&os,progress_seen,&progress,NULL,NULL};
 unsigned pages=99;
 ps.count=2;
 for(int j=0;j<2;j++) {ps.pages[j].height=1;ps.pages[j].width=8;ps.pages[j].bytes_per_line=1;ps.pages[j].bits_per_pixel=1;ps.data[j][0]=(uint8_t)(0xff-j);}
 assert(mac_classic_job_run(&io,&cfg,&pages)==0);
 assert(pages==2 && os.n==122+2*(11+4+1+2)+6);
 assert(progress.n==2 && progress.page[0]==1 && progress.page[1]==2);
 assert(progress.row[0]==0 && progress.row[1]==0);
 assert(progress.percent[0]==0 && progress.percent[1]==0);
 assert(os.data[0]==0x1b && os.data[121]=='@');
 const uint8_t raw[]={0x1b,'D',1,0x16,0xff};
 assert(has(&os,raw,sizeof raw));
 const uint8_t end[]={0x1b,'E',0x1b,'@',0x1b,'Z'};
 assert(memcmp(os.data+os.n-sizeof end,end,sizeof end)==0);

 /* One black RGB24 row goes through the actual recovered DitherPage. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=24,.bits_per_pixel=24,.color_space=1};
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 assert(has(&os,raw,sizeof raw));
 /* The original selects DitherPage by cupsColorSpace == RGB (1), not
  * merely by 24 bits/pixel. Reject a CMY page instead of printing a
  * wrongly interpreted three-channel raster. */
 ps.index=0; ps.pages[0].color_space=4; /* CUPS_CSPACE_CMY */
 init_sink(&os);
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);
 ps.index=0; ps.pages[0].color_space=1;
 init_sink(&os);
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 /* Equally reject a malformed RGB-tagged one-bit source. */
 ps.index=0; ps.pages[0].bits_per_pixel=1;
 init_sink(&os);
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);
 ps.pages[0].bits_per_pixel=24;
  /* Original Draft selects 1-bit CUPS input; RGB24 with Draft is rejected. */
 init_sink(&os);cfg.quality_selector=7;
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);
 cfg.quality_selector=0;

 /* Mode 1: two empty rows must become one ESC f skip. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=3,.bytes_per_line=1,.bits_per_pixel=1};
 ps.data[0][2]=0xff;cfg.optimization_level=1;
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 const uint8_t skip[]={0x1b,'f',0,2,0xff};
 assert(has(&os,skip,sizeof skip));

 /* A nonzero short CUPS read is a hard error, not an EOF. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.truncated=1;
 ps.pages[0]=(mac_classic_page_info){.width=8,.height=2,.bytes_per_line=1,.bits_per_pixel=1};
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);
 /* Truncated output must never be counted as a completed page. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=1,.bits_per_pixel=1};
 ps.data[0][0]=0xff;os.reject_after=123;
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);

 /* In the original, rows 0,32,64,... emit progress; never a mechanical ack. */
 memset(&ps,0,sizeof ps);init_sink(&os);memset(&progress,0,sizeof progress);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=65,.bytes_per_line=1,.bits_per_pixel=1};
 /* Test source has room for 128 bytes and all rows are blank in mode 0. */
 cfg.optimization_level=0;
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 assert(progress.n==3 && progress.row[0]==0 && progress.row[1]==32 && progress.row[2]==64);
 assert(progress.percent[0]==0 && progress.percent[1]==49 && progress.percent[2]==98);
 /* SIGTERM/cancel check interrupts the row stream and blocks end-of-page.
  * No automatic resend of partial printer output is allowed. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=4,.bytes_per_line=1,.bits_per_pixel=1};
 ps.data[0][0]=0xff;ps.data[0][1]=0xff;
 test_cancel cancel={.checks=0,.reject_on=4};
 io.cancelled=cancel_after_n_checks;io.cancel_ctx=&cancel;
 assert(mac_classic_job_run(&io,&cfg,&pages)<0 && pages==0);
 const uint8_t page_end[]={0x1b,'G'};
 assert(!has(&os,page_end,sizeof page_end));
 io.cancelled=NULL;io.cancel_ctx=NULL;

 /* SIGTERM after final raw row must stop before ESC G/E, as the original
  * checks _terminate_flag at 0x100008945 after each ProcessPage row. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=1,.bits_per_pixel=1};
 ps.data[0][0]=0xa5;
 test_cancel after_last={.checks=0,.reject_on=999};
 os.cancel_at_final_row=&after_last;os.final_row_value=0xa5;
 io.cancelled=cancel_after_n_checks;io.cancel_ctx=&after_last;
 assert(mac_classic_job_run(&io,&cfg,&pages)<0 && pages==0);
 assert(!has(&os,page_end,sizeof page_end));
 io.cancelled=NULL;io.cancel_ctx=NULL;

 /* _ProcessPage fourth argument: explicit reprint forces first ESC D for
  * mode 2 when cached width is unchanged (0x100008171..0x10000818b).
  * Normal jobs never enable this branch. */
 {
   uint8_t row[]={0xff}; uint8_t out[32];size_t n=0;
   mac_classic_raster_row st;
   const uint8_t without_force[]={0x1b,'B',0,0x16,0xff};
   const uint8_t with_force[]={0x1b,'B',0,0x1b,'D',1,0x16,0xff};
   assert(mac_classic_row_init(&st,2,1)==0);
   st.previous_width=1;
   assert(mac_classic_row_encode(&st,row,out,sizeof out,&n)==0 &&
          n==sizeof without_force && memcmp(out,without_force,n)==0);
   assert(st.first_nonblank==1);
   assert(mac_classic_row_init(&st,2,1)==0);
   st.previous_width=1;
   mac_classic_row_set_reprint(&st,1);
   assert(mac_classic_row_encode(&st,row,out,sizeof out,&n)==0 &&
          n==sizeof with_force && memcmp(out,with_force,n)==0);
   assert(st.first_nonblank==0);
   assert(mac_classic_row_encode(&st,row,out,sizeof out,&n)==0 &&
          n==sizeof without_force && memcmp(out,without_force,n)==0);
   assert(mac_classic_row_init(&st,2,1)==0);
   st.previous_width=1;
   mac_classic_row_set_reprint(&st,1);
   assert(mac_classic_row_encode(&st,row,out,2,&n)<0 && n==0 &&
          st.first_nonblank==1 && st.previous_width==1);
   assert(mac_classic_row_init(&st,3,1)==0);
   st.previous_width=1;
   mac_classic_row_set_reprint(&st,1);
   const uint8_t mode3_without_force[]={0x16,0xff};
   assert(mac_classic_row_encode(&st,row,out,sizeof out,&n)==0 &&
          n==sizeof mode3_without_force &&
          memcmp(out,mode3_without_force,n)==0);
 }

 /* Original _ReadPage bases the printed row stride on CUPS row bytes,
  * not just page width. Padded 1-bit and complete RGB triple rows must
  * preserve that extra source data rather than be rejected or truncated. */
 {
   unsigned stride=0;
   mac_classic_page_info h={.width=8,.height=1,.bytes_per_line=2,.bits_per_pixel=1};
   assert(mac_classic_packed_stride(&h,&stride)==0 && stride==2);
   h.bits_per_pixel=24; h.bytes_per_line=30;
   assert(mac_classic_packed_stride(&h,&stride)==0 && stride==2);
   h.bytes_per_line=25;
   assert(mac_classic_packed_stride(&h,&stride)<0); /* partial RGB triple */
   h.bytes_per_line=21;
   assert(mac_classic_packed_stride(&h,&stride)<0); /* less than 8 RGB pixels */
   h.bits_per_pixel=1; h.bytes_per_line=121;
   assert(mac_classic_packed_stride(&h,&stride)<0); /* device row byte limit */
 }
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=2,.bits_per_pixel=1};
 ps.data[0][0]=0xff;ps.data[0][1]=0x55;
 cfg.optimization_level=0;
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 const uint8_t mono_padded[]={0x1b,'D',2,0x16,0xff,0x55};
 assert(has(&os,mono_padded,sizeof mono_padded));
 memset(&ps,0,sizeof ps);init_sink(&os);
 ps.count=1;ps.pages[0]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=30,.bits_per_pixel=24,.color_space=1};
 assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==1);
 const uint8_t rgb_padded[]={0x1b,'D',2,0x16,0xff,0xc0};
 assert(has(&os,rgb_padded,sizeof rgb_padded));
 /* The OEM keeps its most recently transmitted ESC D width across
  * _ProcessPage calls: mode 2 in a function-static variable, mode 3
  * in the print descriptor at +0x24. Do not resend it at every page. */
 for (unsigned optimization=2;optimization<=3;++optimization) {
   memset(&ps,0,sizeof ps); init_sink(&os);
   ps.count=2;
   for(unsigned i=0;i<2;++i) {
     ps.pages[i]=(mac_classic_page_info){.width=8,.height=1,.bytes_per_line=1,.bits_per_pixel=1};
     ps.data[i][0]=0xff;
   }
   cfg.optimization_level=optimization;
   assert(mac_classic_job_run(&io,&cfg,&pages)==0 && pages==2);
   const uint8_t width_cmd[]={0x1b,'D',1};
   assert(count_pattern(&os,width_cmd,sizeof width_cmd)==1);
 }
 cfg.optimization_level=0;
 /* No empty 'successful' print job. */
 memset(&ps,0,sizeof ps);init_sink(&os);
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0 && pages==0);
 cfg.vertical_dpi=400;
 assert(mac_classic_job_run(&io,&cfg,&pages)!=0);
 return 0;
}
