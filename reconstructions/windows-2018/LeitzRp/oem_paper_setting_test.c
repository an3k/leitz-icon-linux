#include "oem_paper_setting.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    unsigned calls;
    const uint16_t *name;
    size_t length;
    uint32_t needed;
    uint32_t reported_options;
    int32_t negative_result;
    int unterminated;
} fixture;
static int32_t setting(void *ctx,const char *feature,uint8_t *buf,
                       uint32_t capacity,uint32_t *need,uint32_t *options)
{
    fixture *f=ctx;
    assert(strcmp(feature,"PaperSize")==0 && need && options);
    ++f->calls;
    *need=f->needed;
    *options=f->reported_options;
    if(f->calls%2) {
        assert(!buf && capacity==0);
        return (int32_t)UINT32_C(0x80004005);
    }
    assert(buf && capacity==f->needed);
    const size_t max=f->length+(f->unterminated?0:1);
    assert(2*max<=capacity);
    for(size_t i=0;i<f->length;i++) {
        buf[i*2]=(uint8_t)f->name[i];
        buf[i*2+1]=(uint8_t)(f->name[i]>>8);
    }
    if(!f->unterminated){buf[f->length*2]=0;buf[f->length*2+1]=0;}
    return f->negative_result;
}
int main(void) {
    static const uint16_t normal[]={'P','a','p','e','r','8','8','0',0};
    static const uint16_t continuous[]={'X','-','C','o','n','t','i','n','u','o','u','s','7','0','0',0};
    static const uint16_t custom[]={'X','-','C','U','S','T','O','M','S','I','Z','E',0};
    static const uint16_t both[]={'C','o','n','t','i','n','u','o','u','s','C','U','S','T','O','M','S','I','Z','E',0};
    uint16_t name[64]; size_t n=888;
    leitz_rp_paper_setting_info info={0};
    fixture f={.name=normal,.length=8,.needed=20,.reported_options=2};
    memset(name,0xa5,sizeof name);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    assert(f.calls==2 && info.calls==2 && info.first_needed_bytes==20 && info.last_needed_bytes==20);
    assert(n==8 && name[8]==0 && info.has_paper_name==1);
    assert(info.options_returned==2);
    char ascii[64];assert(leitz_rp_oem_paper_to_ascii(name,n,ascii,sizeof ascii)==0);
    assert(strcmp(ascii,"Paper880")==0);
    f.calls=0;f.name=continuous;f.length=15;f.needed=32;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    f.calls=0;f.name=custom;f.length=12;f.needed=26;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    f.calls=0;f.name=both;f.length=20;f.needed=42;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    /* Negative HRESULT from second call does not gate the original's
     * subsequent string processing (provided data is still valid). */
    f.calls=0;f.negative_result=(int32_t)UINT32_C(0x80004005);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    assert(info.calls==2);
    /* A zero needed length suppresses allocation and second call. */
    f.calls=0;f.needed=0;n=999;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==0);
    assert(f.calls==1 && n==0 && !info.has_paper_name);
    /* No partial publication on odd, oversized or unterminated buffers. */
    f.calls=0;f.needed=41;n=999;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==-1);
    assert(f.calls==1 && n==999);
    f.calls=0;f.needed=42;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,40,name,64,&n,&info)==-1);
    assert(f.calls==1 && n==999);
    f.calls=0;f.unterminated=1;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,&info)==-1);
    assert(f.calls==2 && n==999);
    f.calls=0;f.unterminated=0;
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,10,&n,&info)==-1);
    assert(f.calls==2 && n==999);
    assert(leitz_rp_oem_read_paper_setting(NULL,&f,200,name,64,&n,&info)==-1);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,NULL,64,&n,&info)==-1);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,0,&n,&info)==-1);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,NULL,&info)==-1);
    assert(leitz_rp_oem_read_paper_setting(setting,&f,200,name,64,&n,NULL)==-1);
    uint16_t non_ascii[]={0x20ac,0};
    memset(ascii,'A',sizeof ascii);
    assert(leitz_rp_oem_paper_to_ascii(non_ascii,1,ascii,sizeof ascii)==-1);
    assert(ascii[0]=='A');
    assert(leitz_rp_oem_paper_to_ascii(name,n,ascii,0)==-1);
    puts("OEM PaperSize two-stage GetDriverSetting and bounded UTF16 paper names: PASS");
    return 0;
}
