#include "label_setup.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static leitz_win2018_label_result expect(
    const char *name, uint32_t x, uint32_t y, int orientation,
    uint16_t mode_auto, uint16_t configured,
    uint16_t width, uint16_t height, uint16_t label,
    uint32_t die_cut, uint32_t custom)
{
    leitz_win2018_label_input in = {
        name, x, y, orientation, mode_auto, configured
    };
    leitz_win2018_label_result out = {0};
    leitz_win2018_row_control row = {0};
    row.previous_trimmed_width=123;
    row.label_count=93;
    row.row_index=77;
    row.queued_blank_rows=29;
    row.medium_switch_flag=47;
    assert(leitz_win2018_initialize_label(&in,&out,&row)==0);
    assert(out.paper_width==width && out.paper_height==height);
    assert(out.label_length==label && out.is_die_cut==die_cut);
    assert(out.is_custom_size==custom);
    assert(row.previous_trimmed_width==UINT16_MAX);
    assert(row.label_count==1 && row.row_index==0 &&
           row.queued_blank_rows==0);
    assert(row.label_height==label && row.physical_width==width);
    assert(row.send_width_flag==custom);
    assert(row.medium_switch_flag==47); /* independent OEM medium */
    return out;
}
int main(void)
{
    /* Original 0x18c2/0x18dd wcsstr, 0x1b33..0x1c3b. */
    expect("Continuous70030001",960,1000,1,0,453,
           960,1000,453,0,0);
    /* Non-diecut stock is not rotated at orientation=2. */
    expect("Continuous70030001",960,1000,2,0,453,
           960,1000,453,0,0);
    /* Automatic continuous length uses raster page height. */
    expect("Continuous70030001",960,1000,2,1,453,
           960,1000,1000,0,0);
    /* Fixed stock: page-height governs even in non-auto mode. */
    expect("ShippingLabel70180001",960,1000,1,0,453,
           960,1000,1000,1,0);
    expect("ShippingLabel70180001",960,1000,2,0,453,
           1000,960,960,1,0);
    /* CUSTOMSIZE is recognized as a substring and triggers ESC w. */
    expect("CUSTOMSIZE",960,1000,1,0,453,
           960,1000,1000,1,1);
    expect("CUSTOMSIZE",960,1000,2,0,453,
           1000,960,960,1,1);
    expect("MyCUSTOMSIZELabel",960,1000,2,1,453,
           1000,960,960,1,1);
    /* OEM is case-sensitive: lowercase is NOT Continuous. */
    expect("continuous",960,1000,2,0,453,
           1000,960,960,1,0);
    expect("ExtraContinuousStock",480,11000,1,0,21000,
           480,11000,21000,0,0);
    /* Native state stores 16-bit dimensions: do not invent a
     * universal 170-mm cap or change its truncation semantics. */
    expect("CUSTOMSIZE",70000,72000,2,1,1,
           (uint16_t)72000u,(uint16_t)70000u,(uint16_t)70000u,1,1);
    expect("Continuous112",480,1000,1,0,0,
           480,1000,0,0,0);
    /* Cleanly reject invalid test API pointers without mutation. */
    leitz_win2018_label_result out={0};
    leitz_win2018_row_control row={0};
    leitz_win2018_label_input invalid={0};
    assert(leitz_win2018_initialize_label(NULL,&out,&row)==-1);
    assert(leitz_win2018_initialize_label(&invalid,&out,&row)==-1);
    invalid.paper_name="CUSTOMSIZE";
    assert(leitz_win2018_initialize_label(&invalid,NULL,&row)==-1);
    assert(leitz_win2018_initialize_label(&invalid,&out,NULL)==-1);
    assert(row.label_count==0 && out.paper_width==0);
    puts("Windows 2018 LeitzRp media/ModeAuto/diecut/label-length: PASS");
    return 0;
}
