#include "gpd_job_sequence.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t data[512];
    size_t sizes[24],calls,used;
    size_t fail_call;
} sink;

static int32_t collect(void *ctx,const uint8_t *buffer,size_t size)
{
    sink *s=ctx;
    assert(s->calls<24 && s->used+size<=sizeof s->data);
    s->sizes[s->calls++]=size;
    memcpy(s->data+s->used,buffer,size);
    s->used+=size;
    return s->calls==s->fail_call ? -1:0;
}

static void compare(const sink *s,size_t from,const uint8_t *wanted,size_t size)
{
    assert(from+size<=s->used && !memcmp(s->data+from,wanted,size));
}

int main(void)
{
    sink s={0};
    assert(leitz_win2018_gpd_begin_document(collect,&s)==0);
    assert(s.calls==1 && s.sizes[0]==122);
    for(size_t i=0;i<121;++i) assert(s.data[i]==0x1b);
    assert(s.data[121]=='@');
    assert(leitz_win2018_gpd_setup_page("DHLShippingLabel",
                 LEITZ_WIN2018_RES_OPTION2,collect,&s)==0);
    assert(s.calls==4 && s.sizes[1]==3 && s.sizes[2]==8 && s.sizes[3]==2);
    const uint8_t p1[]={0x1b,'B',0};
    const uint8_t dhl[]={0x1b,'L',0x08,0x36,0x1b,'w',0x03,0xc0};
    const uint8_t r2[]={0x1b,'i'};
    compare(&s,122,p1,sizeof p1);
    compare(&s,125,dhl,sizeof dhl);
    compare(&s,133,r2,sizeof r2);
    const size_t prior=s.calls;
    assert(leitz_win2018_gpd_end_page()==0 && s.calls==prior);
    assert(leitz_win2018_gpd_setup_page("CUSTOMSIZE",
                 LEITZ_WIN2018_RES_OPTION1,collect,&s)==0);
    assert(s.calls==7 && s.sizes[4]==3 && s.sizes[5]==4 && s.sizes[6]==2);
    const uint8_t custom[]={0x1b,'L',0xff,0xff};
    const uint8_t r1[]={0x1b,'h'};
    compare(&s,135,p1,sizeof p1);
    compare(&s,138,custom,sizeof custom);
    compare(&s,142,r1,sizeof r1);
    assert(leitz_win2018_gpd_end_job(collect,&s)==0);
    assert(s.calls==8 && s.sizes[7]==6);
    const uint8_t end[]={0x1b,'E',0x1b,'@',0x1b,'Z'};
    compare(&s,144,end,sizeof end);
    const size_t previous_calls=s.calls, previous_used=s.used;
    assert(leitz_win2018_gpd_setup_page("not-an-option",
                 LEITZ_WIN2018_RES_OPTION1,collect,&s)==-1);
    assert(leitz_win2018_gpd_setup_page("DHLShippingLabel",
                 (leitz_win2018_gpd_resolution)0,collect,&s)==-1);
    assert(leitz_win2018_gpd_setup_page(NULL,
                 LEITZ_WIN2018_RES_OPTION1,collect,&s)==-1);
    assert(leitz_win2018_gpd_setup_page("DHLShippingLabel",
                 LEITZ_WIN2018_RES_OPTION1,NULL,&s)==-1);
    assert(s.calls==previous_calls && s.used==previous_used);
    for(size_t fail=1;fail<=3;fail++) {
        memset(&s,0,sizeof s);
        s.fail_call=fail;
        assert(leitz_win2018_gpd_setup_page("DHLShippingLabel",
                   LEITZ_WIN2018_RES_OPTION1,collect,&s)==-1);
        assert(s.calls==fail);
    }
    memset(&s,0,sizeof s);
    s.fail_call=1;
    assert(leitz_win2018_gpd_begin_document(collect,&s)==-1);
    assert(s.calls==1);
    memset(&s,0,sizeof s);
    s.fail_call=1;
    assert(leitz_win2018_gpd_end_job(collect,&s)==-1);
    assert(s.calls==1);
    puts("Windows GPD static document/page/quality command sequence: PASS");
    return 0;
}
