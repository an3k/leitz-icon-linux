#include "devmode_patch.h"

static uint16_t word(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0]|((uint16_t)p[1]<<8));
}
static uint32_t dword(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|
           ((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void put16(uint8_t *p, uint16_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);
}
static void put32(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);
    p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);
}
/* Windows DEVMODEW: dmSize (+0x44), dmDriverExtra (+0x46),
 * dmFields (+0x48), dmPaperSize (+0x4e), dmPrintQuality (+0x5a),
 * dmYResolution (+0x60). Manufacturer private fields lie at fixed
 * negative offsets from end of dmSize + dmDriverExtra (not from its
 * beginning). Native code does not validate buffer limits; we do.
 */
static int check(uint8_t *p,size_t bytes,const leitz_win2018_devmode_values *v,
                 size_t *end) {
    if(!p||!v||bytes<0x62u) return 0;
    size_t base=word(p+0x44u), extra=word(p+0x46u);
    if(base<0x62u||extra<16u||base>bytes||extra>bytes-base) return 0;
    *end=base+extra;
    return 1;
}
static void common(uint8_t *p,const leitz_win2018_devmode_values *v) {
    put32(p+0x48u,LEITZ_WIN2018_DM_ORIGINAL_FLAGS);
    put16(p+0x4eu,v->paper_size);
    put16(p+0x5au,v->print_quality);
    put16(p+0x60u,v->y_resolution);
}
int leitz_win2018_patch_set_paper(uint8_t *p,size_t bytes,
                                  const leitz_win2018_devmode_values *v) {
    size_t end=0;
    if(!check(p,bytes,v,&end))return -1;
    /* PrintHelper.dll 0x1c43: test BYTE [devmode+0x48],2;
     * branch to failure without mutating when DM_PAPERSIZE not set. */
    if(!(dword(p+0x48u)&LEITZ_WIN2018_DM_PAPERSIZE))return 0;
    common(p,v);
    put16(p+end-0x10u,v->private_minus_16);
    put16(p+end-0x0cu,1u); /* OEM 0x1c90: fixed mode flag */
    put16(p+end-0x0au,v->private_minus_10);
    return 1;
}
int leitz_win2018_patch_custom_paper(uint8_t *p,size_t bytes,
                                     const leitz_win2018_devmode_values *v) {
    size_t end=0;
    if(!check(p,bytes,v,&end))return -1;
    /* PrintHelper.dll 0x1a42..0x1a8d: no paper-size gate,
     * writes three distinct private fields, including -0x0e. */
    common(p,v);
    put16(p+end-0x10u,v->private_minus_16);
    put16(p+end-0x0eu,v->private_minus_14);
    put16(p+end-0x0au,v->private_minus_10);
    return 1;
}
