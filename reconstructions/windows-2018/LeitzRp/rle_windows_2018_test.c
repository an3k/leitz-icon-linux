#include "rle_windows_2018.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void vector(const uint8_t *raw, size_t n, const uint8_t *expected,
                   size_t expected_size, int status)
{
    uint8_t out[2048]={0};
    size_t written=99;
    assert(n <= sizeof out);
    int actual=leitz_win2018_rle_encode(raw,n,out,sizeof out,&written);
    if (actual!=status ||
        (actual==0 && (written!=expected_size ||
                       memcmp(out,expected,written))) ||
        (actual==1 && written!=0)) {
        fprintf(stderr,"RLE vector failure n=%zu returned=%d expected=%d "
                       "written=%zu expected_size=%zu\n",
                n,actual,status,written,expected_size);
        abort();
    }
}

/* Distinct test-only bitwise reference: no optimized 00/ff-byte shortcut.
 * Comparison proves the byte shortcut matches canonical run boundaries.
 */
static size_t bit_reference(const uint8_t *raw, size_t n, uint8_t *out)
{
    unsigned bit=0,run=0; size_t used=0;
    for(size_t i=0;i<n;++i)for(unsigned mask=0x80;mask;mask>>=1){
        unsigned b=!!(raw[i]&mask);
        if(run && (b!=bit || run==128)){
            out[used++]=(uint8_t)((bit<<7)|(run-1));run=0;
        }
        if(!run)bit=b;
        ++run;
    }
    if(run)out[used++]=(uint8_t)((bit<<7)|(run-1));
    return used;
}

int main(void)
{
    static const uint8_t z32[32]={0};
    static const uint8_t z16[16]={0};
    static const uint8_t f4[4]={0xff,0xff,0xff,0xff};
    static const uint8_t mixed[6]={0,0,0xff,0xff,0,0};
    const uint8_t a[]={0x7f,0x7f}, b[]={0x7f}, c[]={0x9f};
    const uint8_t d[]={0x0f,0x8f,0x0f};
    const uint8_t noisy[]={0xaa,0x55};
    vector(z32,sizeof z32,a,sizeof a,0);
    vector(z16,sizeof z16,b,sizeof b,0);
    vector(f4,sizeof f4,c,sizeof c,0);
    vector(mixed,sizeof mixed,d,sizeof d,0);
    vector(noisy,sizeof noisy,NULL,0,1);
    const uint8_t zero[]={0};
    vector(zero,1,NULL,0,1);
    vector(NULL,0,NULL,0,1);
    const uint8_t ones17[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    const uint8_t e[]={0xff,0x87};
    vector(ones17,sizeof ones17,e,sizeof e,0);
    size_t m=1;uint8_t dest[1024];
    assert(leitz_win2018_rle_encode(z32,sizeof z32,dest,31,&m)==-1);
    assert(leitz_win2018_rle_encode(NULL,3,dest,sizeof dest,&m)==-1);
    assert(leitz_win2018_rle_encode(z32,sizeof z32,dest,sizeof dest,NULL)==-1);

    /* Deterministic corpus across run lengths, changes, and bit patterns.
     * These are source-derived vectors, not native DLL runtime parity. */
    uint32_t seed=0x372bf123u;
    for(size_t len=0;len<=96;++len){
        for(int mode=0;mode<25;++mode){
            uint8_t src[128]={0};uint8_t expected[2048]={0};
            for(size_t i=0;i<len;++i){
                seed=seed*1664525u+1013904223u;
                src[i]=(uint8_t)(mode==0?0:
                         (mode==1?255:(mode==2?(i%2?255:0):seed>>24)));
            }
            size_t rn=bit_reference(src,len,expected);
            if(rn<len)vector(src,len,expected,rn,0);
            else vector(src,len,NULL,0,1);
        }
    }
    {
        const uint8_t raw[]={0xff,0xff,0xff,0xff};
        uint8_t clipped[4]={0};
        assert(leitz_win2018_copy_clipped_row(raw,3,17,clipped,4)==0);
        assert(clipped[0]==0xff && clipped[1]==0xff && clipped[2]==0x80);
        assert(raw[2]==0xff);
        assert(leitz_win2018_copy_clipped_row(raw,3,16,clipped,4)==0);
        assert(clipped[2]==0xff);
        assert(leitz_win2018_copy_clipped_row(raw,3,24,clipped,4)==0);
        assert(clipped[2]==0xff);
        assert(leitz_win2018_copy_clipped_row(raw,1,1,clipped,4)==0);
        assert(clipped[0]==0x80);
        assert(leitz_win2018_copy_clipped_row(raw,1,0,clipped,4)==-1);
        assert(leitz_win2018_copy_clipped_row(raw,3,17,clipped,2)==-1);
    }
    puts("Windows LeitzRp 2018 RLE and safe last-byte clipping: PASS");
    return 0;
}
