#include "rle_windows_2018.h"
#include "filter_graphics_fragments.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* New C test API, not a Windows COM/Unidrv runtime test. Emission
 * boundaries match the manufacturer's separate WriteData calls. */
typedef struct {
    uint8_t bytes[512];
    size_t written, calls, lengths[16];
} mock;
static int32_t capture(void *ctx,const uint8_t *data,size_t n)
{
    mock*m=ctx;
    assert(m->calls<16 && m->written+n<=sizeof m->bytes);
    m->lengths[m->calls++]=n;
    memcpy(m->bytes+m->written,data,n);
    m->written+=n;
    return 0;
}
static void compare(const mock*m,const uint8_t *reference,
                    size_t n,size_t calls)
{
    assert(m->written==n && m->calls==calls);
    assert(memcmp(m->bytes,reference,n)==0);
}
int main(void)
{
    mock m={0}; leitz_win2018_row_state s={0};
    uint8_t compressed[64];size_t encoded=0;
    const uint8_t a[]={0,0,0xff,0xff,0xff,0xff};
    assert(leitz_win2018_emit_row_prefix(&s,sizeof a,2,capture,&m)==0);
    assert(leitz_win2018_rle_encode(a+2,4,compressed,sizeof compressed,&encoded)==0);
    assert(encoded==1 && compressed[0]==0x9f);
    assert(leitz_win2018_emit_row_payload(a+2,4,compressed,encoded,capture,&m)==0);
    const uint8_t a_out[]={0x1b,'D',4,0x1b,'B',2,0x17,0x9f};
    compare(&m,a_out,sizeof a_out,4);
    assert(m.lengths[0]==3 && m.lengths[1]==3 &&
           m.lengths[2]==1 && m.lengths[3]==1);

    /* The already-recovered WriteData helper compares raw and
     * compressed lengths. For USE_RAW pass the raw-size candidate
     * to preserve its exact original predicate (NOT a null/0 pair). */
    memset(&m,0,sizeof m);
    const uint8_t b[]={0,0,0xaa,0x55,0xaa,0x55};
    assert(leitz_win2018_emit_row_prefix(&s,sizeof b,2,capture,&m)==0);
    assert(leitz_win2018_rle_encode(b+2,4,compressed,sizeof compressed,&encoded)==1);
    assert(encoded==0);
    assert(leitz_win2018_emit_row_payload(b+2,4,b+2,4,capture,&m)==0);
    const uint8_t b_out[]={0x1b,'B',2,0x16,0xaa,0x55,0xaa,0x55};
    compare(&m,b_out,sizeof b_out,3);
    assert(m.lengths[0]==3 && m.lengths[1]==1 && m.lengths[2]==4);

    /* Original x64 clips unused final low bits at certain full-width
     * row boundaries. The portable helper copies source before masking. */
    memset(&m,0,sizeof m);s.previous_trimmed_width=0;
    const uint8_t c[]={0xff,0xff,0xff,0xff,0xff};
    uint8_t copy[5]={0};
    assert(leitz_win2018_copy_clipped_row(c,5,33,copy,sizeof copy)==0);
    assert(c[4]==0xff && copy[4]==0x80);
    assert(leitz_win2018_emit_row_prefix(&s,5,0,capture,&m)==0);
    assert(leitz_win2018_rle_encode(copy,5,compressed,sizeof compressed,&encoded)==0);
    assert(encoded==2);
    assert(leitz_win2018_emit_row_payload(copy,5,compressed,encoded,capture,&m)==0);
    const uint8_t c_out[]={0x1b,'D',5,0x1b,'B',0,0x17,0xa0,0x06};
    compare(&m,c_out,sizeof c_out,4);
    puts("Windows LeitzRp 2018 prefix + RLE + selector integration: PASS");
    return 0;
}
