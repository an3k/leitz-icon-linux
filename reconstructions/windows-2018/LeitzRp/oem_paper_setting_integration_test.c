#include "oem_paper_setting.h"
#include "label_setup.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const uint16_t *text;
    size_t n;
    unsigned calls;
} original_host;
static int32_t original_get_setting(void *context, const char *feature,
    uint8_t *dst, uint32_t capacity, uint32_t *required, uint32_t *options)
{
    original_host *host=context;
    assert(strcmp(feature,"PaperSize")==0);
    ++host->calls;
    *required=(uint32_t)((host->n+1u)*2u);
    *options=1;
    if(!dst) {
        assert(capacity==0);
        return (int32_t)UINT32_C(0x80004005);
    }
    assert(capacity==*required);
    for(size_t i=0;i<host->n;i++) {
        dst[2*i]=(uint8_t)host->text[i];
        dst[2*i+1]=(uint8_t)(host->text[i]>>8);
    }
    dst[2*host->n]=0;
    dst[2*host->n+1]=0;
    return 0;
}
static void verify(const uint16_t *original, size_t count,
                   uint32_t diecut, uint32_t custom, uint16_t length)
{
    original_host host={.text=original,.n=count};
    uint16_t text[96]={0};char option[96]={0};size_t got=0;
    leitz_rp_paper_setting_info info={0};
    assert(leitz_rp_oem_read_paper_setting(original_get_setting,&host,
                 sizeof text,text,96,&got,&info)==0);
    assert(host.calls==2 && got==count && info.calls==2);
    assert(leitz_rp_oem_paper_to_ascii(text,got,option,sizeof option)==0);
    leitz_win2018_label_input setup={option,300,600,1,0,137};
    leitz_win2018_label_result result={0};
    leitz_win2018_row_control row={0};
    assert(leitz_win2018_initialize_label(&setup,&result,&row)==0);
    assert(result.is_die_cut==diecut);
    assert(result.is_custom_size==custom);
    assert(result.label_length==length);
    assert(row.label_height==length && row.physical_width==300);
}
int main(void)
{
    static const uint16_t diecut[]={ 'C','a','r','t','r','i','d','g','e','8','8',0 };
    static const uint16_t endless[]={ 'C','o','n','t','i','n','u','o','u','s','7','0','0','3','0','0','0','1',0 };
    static const uint16_t custom[]={ 'C','U','S','T','O','M','S','I','Z','E',0 };
    verify(diecut,11,1,0,600);
    verify(endless,18,0,0,137);
    verify(custom,10,1,1,600);
    puts("Original Windows PaperSize -> existing label_setup (diecut/endless/custom): PASS");
    return 0;
}
