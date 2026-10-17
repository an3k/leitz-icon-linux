#include "gpd_media_catalog.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    assert(leitz_win2018_gpd_media_count() == 77u);
    unsigned continuous = 0, abnormal = 0, fixed = 0;
    for (size_t i = 0; i < leitz_win2018_gpd_media_count(); ++i) {
        const leitz_win2018_gpd_media_entry *m = leitz_win2018_gpd_media_at(i);
        assert(m && m->original_id);
        assert(leitz_win2018_gpd_media_by_name(m->original_id) == m);
        assert(m->command[0] == 0x1b && m->command[1] == 'L');
        if (m->page_dimensions[0] < 0) {
            assert(!strcmp(m->original_id, "CUSTOMSIZE"));
            assert(m->option_id == -1 && m->resource_name_id == -1);
            assert(m->command_length == 4u);
            assert(m->command[2] == 0xff && m->command[3] == 0xff);
            assert(m->min_size[0] == 16 && m->min_size[1] == 16);
            assert(m->max_size[0] == 10000 && m->max_size[1] == 64000);
            continue;
        }
        ++fixed;
        assert(m->option_id >= 259 && m->option_id <= 335);
        assert(m->resource_name_id == m->option_id);
        assert(leitz_win2018_gpd_media_by_option_id(m->option_id) == m);
        assert(m->command_length == 8u);
        assert(m->command[4] == 0x1b && m->command[5] == 'w');
        const unsigned width = (unsigned)m->command[6] * 256u + m->command[7];
        const unsigned length = (unsigned)m->command[2] * 256u + m->command[3];
        assert(width == (unsigned)m->printable_area[0]);
        if (m->page_dimensions[1] == 64000) {
            ++continuous;
            assert(length == 65535u); /* no page-height cap */
        } else if (length * 2u != (unsigned)m->page_dimensions[1]) {
            ++abnormal;
            assert((!strcmp(m->original_id, "AppointmentCard") ||
                    !strcmp(m->original_id, "LeitzAppointmentCard")));
            assert(length == 570u && m->page_dimensions[1] == 1200);
        }
        uint8_t bytes[9];
        size_t written=91;
        memset(bytes,0xcc,sizeof bytes);
        assert(leitz_win2018_gpd_media_copy_command(m,bytes,7,&written)==-1);
        assert(bytes[0]==0xcc && written==91);
        assert(leitz_win2018_gpd_media_copy_command(m,bytes,8,&written)==0);
        assert(written==8 && !memcmp(bytes,m->command,8) && bytes[8]==0xcc);
    }
    assert(fixed == 76u && continuous == 20u && abnormal == 2u);
    const leitz_win2018_gpd_media_entry *dhl =
        leitz_win2018_gpd_media_by_name("DHLShippingLabel");
    const uint8_t dhlbytes[]={0x1b,'L',0x08,0x36,0x1b,'w',0x03,0xc0};
    assert(dhl && dhl->option_id==335 && dhl->resource_name_id==335);
    assert(!memcmp(dhl->command,dhlbytes,sizeof dhlbytes));
    assert(dhl->page_dimensions[0]==1039 && dhl->page_dimensions[1]==4204);
    assert(!leitz_win2018_gpd_media_by_name("invalid"));
    assert(!leitz_win2018_gpd_media_by_name(NULL));
    assert(!leitz_win2018_gpd_media_at(77));
    assert(!leitz_win2018_gpd_media_by_option_id(-1));
    assert(!leitz_win2018_gpd_media_by_option_id(999));
    puts("Windows 2018 all 77 original GPD PaperSize records: PASS");
    return 0;
}
