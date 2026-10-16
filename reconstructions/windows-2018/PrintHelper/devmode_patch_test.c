#include "devmode_patch.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void put16(uint8_t *p,uint16_t n) {p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
static void put32(uint8_t *p,uint32_t n) {p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);p[2]=(uint8_t)(n>>16);p[3]=(uint8_t)(n>>24);}
static void verify(const uint8_t *p,size_t end,int is_custom) {
    assert(p[0x48]==0x02 && p[0x49]==0x24 && p[0x4a]==0 && p[0x4b]==0);
    assert(p[0x4e]==0x35 && p[0x4f]==0x12);
    assert(p[0x5a]==0x78 && p[0x5b]==0x56);
    assert(p[0x60]==0xbc && p[0x61]==0x9a);
    assert(p[end-16]==0xf0 && p[end-15]==0xde);
    assert(p[end-10]==0x34 && p[end-9]==0x12);
    if(is_custom){assert(p[end-14]==0x9a && p[end-13]==0xbc);}
    else {assert(p[end-12]==1 && p[end-11]==0);}
}
int main(void) {
    uint8_t p[256],before[256];
    leitz_win2018_devmode_values v={0x1235,0x5678,0x9abc,0xdef0,0xbc9a,0x1234};
    memset(p,0xcc,sizeof p);
    put16(p+0x44,0xdc); put16(p+0x46,0x20);
    put32(p+0x48,0x80000002u);
    assert(leitz_win2018_patch_set_paper(p,sizeof p,&v)==1);
    verify(p,0xfc,0);
    assert(p[0]==0xcc && p[255]==0xcc && p[0x50]==0xcc);
    assert(p[0xfc-14]==0xcc);
    memset(p,0xcc,sizeof p);
    put16(p+0x44,0xdc); put16(p+0x46,0x20);
    put32(p+0x48,0x80000000u);memcpy(before,p,sizeof p);
    assert(leitz_win2018_patch_set_paper(p,sizeof p,&v)==0);
    assert(!memcmp(p,before,sizeof p));
    assert(leitz_win2018_patch_custom_paper(p,sizeof p,&v)==1);
    verify(p,0xfc,1);
    assert(p[0xfc-12]==0xcc);
    assert(p[0]==0xcc && p[255]==0xcc);
    memcpy(before,p,sizeof p);
    assert(leitz_win2018_patch_custom_paper(p,251,&v)==-1);
    assert(!memcmp(p,before,sizeof p));
    put16(p+0x46,15);memcpy(before,p,sizeof p);
    assert(leitz_win2018_patch_custom_paper(p,sizeof p,&v)==-1);
    assert(!memcmp(p,before,sizeof p));
    put16(p+0x46,0x20);put16(p+0x44,0x40);memcpy(before,p,sizeof p);
    assert(leitz_win2018_patch_custom_paper(p,sizeof p,&v)==-1);
    assert(!memcmp(p,before,sizeof p));
    assert(leitz_win2018_patch_set_paper(NULL,0,&v)==-1);
    assert(leitz_win2018_patch_set_paper(p,sizeof p,NULL)==-1);
    puts("Windows PrintHelper 2018 SetPaper/SetCustomPaper DEVMODE patch: PASS");
}
