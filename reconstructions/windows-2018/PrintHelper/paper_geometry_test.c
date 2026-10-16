#include "paper_geometry.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef struct {int indices[20];size_t reads,selects,fail_at;uint16_t paper;} fixture;
static int choose(void *p,uint16_t id) {
  fixture *f=p;f->paper=id;++f->selects;return 0;
}
static int query(void *p,int idx,int32_t *v) {
  fixture *f=p;
  assert(f->reads<20);
  f->indices[f->reads++]=idx;
  if(f->fail_at && f->reads==f->fail_at)return -1;
  *v=(int32_t)idx*100+f->paper;
  return 0;
}
int main(void) {
  /* 335 is vendor DHLShippingLabel; 7 is a synthetic test ID. */
  const uint16_t paper[]={335,7};
  const int expected[]={112,113,110,111,8,10,112,113,110,111,8,10};
  leitz_win2018_paper_geometry results[2]={{0},{0}};
  fixture f={0};
  assert(leitz_win2018_collect_paper_geometry(
    paper,2,results,2,choose,query,&f)==0);
  assert(f.selects==2 && f.reads==12);
  assert(!memcmp(f.indices,expected,sizeof expected));
  assert(results[0].paper_id==335 && results[1].paper_id==7);
  assert(results[0].physical_offset_x==11535);
  assert(results[0].physical_offset_y==11635);
  assert(results[0].physical_width==11335);
  assert(results[0].physical_height==11435);
  assert(results[0].printable_raster_width==1135);
  assert(results[0].printable_raster_height==1335);
  assert(results[1].printable_raster_height==1007);
  memset(&f,0,sizeof f);f.fail_at=8;
  memset(results,0xa5,sizeof results);
  leitz_win2018_paper_geometry before[2];
  memcpy(before,results,sizeof results);
  assert(leitz_win2018_collect_paper_geometry(
    paper,2,results,2,choose,query,&f)==-1);
  assert(results[0].paper_id==335);
  assert(!memcmp(&results[1],&before[1],sizeof results[1]));
  memset(&f,0,sizeof f);
  assert(leitz_win2018_collect_paper_geometry(
    paper,2,results,1,choose,query,&f)==-1);
  assert(f.selects==0);
  assert(leitz_win2018_collect_paper_geometry(
    NULL,0,NULL,0,NULL,NULL,NULL)==0);
  assert(leitz_win2018_collect_paper_geometry(
    NULL,2,results,2,choose,query,&f)==-1);
  puts("Windows 2018 PrintHelper GDI geometry: PASS");
  return 0;
}
