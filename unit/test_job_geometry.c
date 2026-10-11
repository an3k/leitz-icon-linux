/* Per-job width/resolution are shared media policy, never codec features. */
#include "../../src/leitz_jobs.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void label(leitz_job *job, unsigned height) {
  uint8_t pixels[16]={0};
  assert(leitz_job_begin_page(job,16,height+2,height,1,1,65535,600)==0);
  for(unsigned y=0;y<height;y++){
    memset(pixels,0,sizeof pixels);
    if(y==0)pixels[0]=1;
    assert(leitz_job_row(job,pixels)==0);
  }
  assert(leitz_job_end_page(job)==0);
}
static void accepted(leitz_protocol protocol,int each) {
  FILE *f=tmpfile();
  assert(f);
  leitz_job *job=leitz_job_create(f,protocol,each);
  assert(job);
  label(job,2);
  label(job,4); /* Same band width; independently chosen label length. */
  assert(leitz_job_finish(job)==0);
  char why[160]={0};
  assert(leitz_job_validate(job,why,sizeof why)==0);
  assert(leitz_job_page_count(job)==2);
  leitz_job_destroy(job);
  fclose(f);
}
static void rejected(leitz_protocol protocol,unsigned width,unsigned dpi) {
  FILE *f=tmpfile();
  assert(f);
  leitz_job *job=leitz_job_create(f,protocol,1);
  assert(job);
  label(job,2);
  assert(leitz_job_begin_page(job,width,4,2,1,1,65535,dpi)==-1);
  leitz_job_destroy(job);
  fclose(f);
}
int main(void) {
  for(int i=0;i<2;i++){
    leitz_protocol proto=i?LEITZ_PROTOCOL_NATIVE:LEITZ_PROTOCOL_CURRENT;
    accepted(proto,0);
    accepted(proto,1);
    rejected(proto,24,600);
    rejected(proto,16,300);
  }
  puts("PASS both codecs share fixed width/dpi and variable label lengths");
  return 0;
}
