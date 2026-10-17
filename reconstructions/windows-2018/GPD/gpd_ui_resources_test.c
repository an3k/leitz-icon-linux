#include "gpd_ui_resources.h"
#include "gpd_media_catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    size_t fixed=0;
    for(size_t i=0;i<leitz_win2018_gpd_media_count();++i){
        const leitz_win2018_gpd_media_entry *m=leitz_win2018_gpd_media_at(i);
        const char *label=leitz_win2018_paper_display_name(m->original_id);
        if(m->resource_name_id<0) {
            assert(!strcmp(m->original_id,"CUSTOMSIZE"));
            assert(!label);continue;
        }
        ++fixed;
        assert(label && *label);
        assert(label==leitz_win2018_resource_string((uint32_t)m->resource_name_id));
        assert(m->resource_name_id>=259 && m->resource_name_id<=335);
        assert(m->resource_name_id!=333);
    }
    assert(fixed==76u);
    assert(!strcmp(leitz_win2018_paper_display_name("DHLShippingLabel"),
                   "*Package Label 7\"x3.5\""));
    assert(!strcmp(leitz_win2018_paper_display_name("NameBadgeCard8214"),
                   "Name Badge Card - Durable\xc2\xa9 8214"));
    assert(!strcmp(leitz_win2018_resource_string(333),"*Custom paper size"));
    assert(leitz_win2018_paper_display_name("CUSTOMSIZE")==NULL);
    assert(leitz_win2018_paper_display_name("missing")==NULL);
    assert(leitz_win2018_paper_display_name(NULL)==NULL);
    assert(leitz_win2018_resource_string(258)==NULL);
    assert(leitz_win2018_resource_string(336)==NULL);
    puts("Windows OEM x86/x64 resource labels for all 76 fixed GPD media: PASS");
    return 0;
}
