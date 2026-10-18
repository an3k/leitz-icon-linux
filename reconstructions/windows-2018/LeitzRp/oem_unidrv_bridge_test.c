#include "oem_unidrv_bridge.h"
#include "oem_method_map.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned invalid, mismatch, releases; void *last; } trace;
static void last_error(void *ctx, uint32_t error) {
    trace *t = ctx;
    assert(error == 87u);
    ++t->invalid;
}
static void mismatch(void *ctx) { ++((trace*)ctx)->mismatch; }
static void release(void *ctx, void *interface) {
    trace *t=ctx;
    ++t->releases;
    t->last=interface;
}
static void put32(uint8_t *p,uint32_t n) {
    for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(n>>(8u*i));
}
static uint32_t get32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 |
           (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
int main(void) {
    const leitz_rp_method_entry *s4=leitz_rp_oem_method_at(4);
    const leitz_rp_method_entry *s6=leitz_rp_oem_method_at(6);
    assert(s4 && s4->x86_rva==0x15d0 && s4->x64_rva==0x15f0);
    assert(s6 && s6->x86_rva==0x1340 && s6->x64_rva==0x1320);
    trace t={0};
    leitz_win2018_oemdm_callbacks callbacks={last_error,mismatch,&t};
    uint8_t destination[40],source[40];
    memset(destination,0xa5,sizeof destination);
    memset(source,0x9c,sizeof source);
    uint32_t published=97;
    leitz_rp_oem_unidrv_devmode args={
        .operation=LEITZ_WIN2018_OEMDM_SIZE,
        .source=source,.source_capacity=sizeof source,
        .destination=destination,.destination_capacity=sizeof destination,
        .published_size=&published,.callbacks=&callbacks
    };
    assert(leitz_rp_oem_unidrv_slot4_devmode(&args)==0 && published==28);
    for(size_t i=0;i<sizeof destination;i++)assert(destination[i]==0xa5);
    args.operation=LEITZ_WIN2018_OEMDM_DEFAULT;
    assert(leitz_rp_oem_unidrv_slot4_devmode(&args)==0);
    assert(get32(destination)==28 && get32(destination+4)==0x4c504944u);
    assert(get32(destination+8)==1 && get32(destination+12)==1);
    assert(get32(destination+16)==0x10000u);
    for(size_t i=20;i<sizeof destination;i++) assert(destination[i]==0xa5);
    put32(source,28); put32(source+4,0x4c504944u);
    put32(source+8,0xbccdaaff);put32(source+12,17);
    args.operation=LEITZ_WIN2018_OEMDM_CONVERT;
    assert(leitz_rp_oem_unidrv_slot4_devmode(&args)==0);
    assert(get32(destination+8)==1 && get32(destination+12)==17);
    args.operation=LEITZ_WIN2018_OEMDM_MERGE;
    put32(source+4,0x11223344);
    assert(leitz_rp_oem_unidrv_slot4_devmode(&args)==0);
    assert(t.mismatch==1 && get32(destination+4)==0x4c504944u);
    args.operation=(leitz_win2018_oemdm_mode)5;
    assert((uint32_t)leitz_rp_oem_unidrv_slot4_devmode(&args)==0x80004005u);
    assert(t.invalid==1);
    assert((uint32_t)leitz_rp_oem_unidrv_slot4_devmode(NULL)==0x80004005u);

    int host=17;
    leitz_rp_oem_binding binding={&host};
    assert(leitz_rp_oem_unidrv_slot6_reset(&binding,release,&t)==LEITZ_RP_S_OK);
    assert(binding.driver_interface==NULL && t.releases==1 && t.last==&host);
    assert(leitz_rp_oem_unidrv_slot6_reset(&binding,release,&t)==LEITZ_RP_S_OK);
    assert(t.releases==1);
    binding.driver_interface=&host;
    assert(leitz_rp_oem_unidrv_slot6_reset(&binding,NULL,&t)==LEITZ_RP_E_POINTER);
    assert(binding.driver_interface==&host && t.releases==1);
    assert(leitz_rp_oem_unidrv_slot6_reset(NULL,release,&t)==LEITZ_RP_E_POINTER);
    assert(leitz_rp_oem_unidrv_slot6_reset(&binding,release,&t)==LEITZ_RP_S_OK);
    assert(t.releases==2 && binding.driver_interface==NULL);
    puts("OEM raster slot 4 original DevMode import + slot 6 host release: PASS");
    return 0;
}
