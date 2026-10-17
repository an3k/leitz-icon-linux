#include "oem_devmode.h"
#include <string.h>
static uint32_t rd32(const uint8_t *p) {
 return (uint32_t)p[0] | ((uint32_t)p[1]<<8) |
        ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static void wr32(uint8_t *p,uint32_t v) {
 p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8);
 p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
static int32_t fail(const leitz_win2018_oemdm_callbacks *cb) {
 if(cb && cb->set_last_error) cb->set_last_error(cb->context,87u);
 return LEITZ_WIN2018_OEMDEVMODE_E_FAIL;
}
static void init(uint8_t *d) {
 /* Native x86 RVA 0x10c1 and x64 RVA 0x10c0: first 20 bytes only. */
 wr32(d,28u); wr32(d+4,LEITZ_WIN2018_OEMDEVMODE_SIGNATURE);
 wr32(d+8,1u); wr32(d+12,1u); wr32(d+16,0x10000u);
}
int32_t leitz_win2018_oemdevmode(leitz_win2018_oemdm_mode mode,
 const uint8_t *src,size_t source_capacity,uint8_t *dst,size_t destination_capacity,
 uint32_t *size_bytes,const leitz_win2018_oemdm_callbacks *cb) {
 if(mode<LEITZ_WIN2018_OEMDM_SIZE||mode>LEITZ_WIN2018_OEMDM_MERGE)
  return fail(cb);
 if(mode==LEITZ_WIN2018_OEMDM_SIZE) {
  if(!size_bytes)return fail(cb);
  *size_bytes=28u;return 0;
 }
 if(!dst||destination_capacity<28u)return fail(cb);
 if(mode==LEITZ_WIN2018_OEMDM_DEFAULT){init(dst);return 0;}
 /* Both native modes 3/4 call the same helper, x86 RVA 0x1000. */
 if(!src||source_capacity<12u)return fail(cb);
 if(rd32(src+4)!=rd32(dst+4)) {
  if(cb&&cb->signature_mismatch)cb->signature_mismatch(cb->context);
  init(dst);return 0;
 }
 size_t count=rd32(src);
 size_t target=rd32(dst);
 if(count>target)count=target;
 if(count>source_capacity||count>destination_capacity)return fail(cb);
 /* Defined on overlap unlike OEM memcpy; this is a safety adaptation. */
 if(count)memmove(dst,src,count);
 wr32(dst,28u);wr32(dst+8,1u);return 0;
}
