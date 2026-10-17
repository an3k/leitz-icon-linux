#include "oem_devmode.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {int invalid,mismatch;uint32_t code;} trace;
static void bad(void *ctx,uint32_t code) {
 trace *t=ctx;t->invalid++;t->code=code;
}
static void note(void *ctx) {((trace *)ctx)->mismatch++;}
static void wr32(uint8_t *p,uint32_t v) {
 for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8*i));
}
static uint32_t rd32(const uint8_t *p) {
 return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
int main(void) {
 trace t={0};leitz_win2018_oemdm_callbacks cb={bad,note,&t};
 uint8_t src[40],dst[40],before[40];uint32_t n=99;
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_SIZE,NULL,0,NULL,0,&n,&cb)==0);
 assert(n==28);
 memset(dst,0xa5,sizeof dst);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_DEFAULT,NULL,0,dst,sizeof dst,NULL,&cb)==0);
 const uint8_t expected[20]={28,0,0,0,'D','I','P','L',1,0,0,0,1,0,0,0,0,0,1,0};
 assert(!memcmp(dst,expected,20));
 for(size_t i=20;i<sizeof dst;i++)assert(dst[i]==0xa5);
 memset(src,0x31,sizeof src);wr32(src,28);wr32(src+4,0x4c504944);
 wr32(src+8,0xaabbccdd);wr32(src+12,0x76543210);
 memset(dst,0x55,sizeof dst);wr32(dst,28);wr32(dst+4,0x4c504944);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_CONVERT,src,sizeof src,dst,sizeof dst,NULL,&cb)==0);
 assert(rd32(dst)==28 && rd32(dst+8)==1 && rd32(dst+12)==0x76543210);
 assert(!memcmp(dst+12,src+12,16)&&dst[28]==0x55);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_MERGE,src,sizeof src,dst,sizeof dst,NULL,&cb)==0);
 memset(dst,0xc3,sizeof dst);wr32(dst,20);wr32(dst+4,0x4c504944);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_CONVERT,src,sizeof src,dst,sizeof dst,NULL,&cb)==0);
 assert(rd32(dst)==28&&rd32(dst+8)==1&&dst[20]==0xc3);
 memset(dst,0xa5,sizeof dst);wr32(dst,28);wr32(dst+4,0x11223344);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_MERGE,src,sizeof src,dst,sizeof dst,NULL,&cb)==0);
 assert(t.mismatch==1&&!memcmp(dst,expected,20));
 for(size_t i=20;i<sizeof dst;i++)assert(dst[i]==0xa5);
 /* Original compares signatures only with each other, not magic. */
 memset(src,0x63,sizeof src);wr32(src,28);wr32(src+4,0x12345678);
 memset(dst,0xc1,sizeof dst);wr32(dst,28);wr32(dst+4,0x12345678);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_CONVERT,src,sizeof src,dst,sizeof dst,NULL,&cb)==0);
 assert(rd32(dst+4)==0x12345678 && rd32(dst+8)==1);
 assert(leitz_win2018_oemdevmode(0,src,40,dst,40,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 assert(leitz_win2018_oemdevmode(5,src,40,dst,40,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_SIZE,NULL,0,NULL,0,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 memset(dst,0xb0,sizeof dst);memcpy(before,dst,sizeof dst);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_DEFAULT,NULL,0,dst,19,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 assert(!memcmp(dst,before,sizeof dst));
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_CONVERT,NULL,0,dst,40,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 assert(!memcmp(dst,before,sizeof dst));
 memset(src,0,sizeof src);wr32(src,100);wr32(src+4,0xb0b0b0b0);
 assert(leitz_win2018_oemdevmode(LEITZ_WIN2018_OEMDM_MERGE,src,24,dst,40,NULL,&cb)==LEITZ_WIN2018_OEMDEVMODE_E_FAIL);
 assert(!memcmp(dst,before,sizeof dst));
 assert(t.invalid==6&&t.code==87);
 puts("Windows LeitzDevMode OEMDevMode modes 1..4: PASS");
 return 0;
}
