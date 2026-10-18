#include "oem_gdi_caps.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int created, deleted, returns_null, last_destroy_value;
    unsigned probes;
    const void *devmode_seen;
    int32_t indices[LEITZ_WIN2018_GDI_CAP_COUNT];
    int32_t returned[LEITZ_WIN2018_GDI_CAP_COUNT];
    uint16_t expected_printer[3];
} fake;

static void *make_dc(void *context, const uint16_t *driver,
    const uint16_t *printer, const uint16_t *port, const void *devmode)
{
    fake *f=context;
    static const uint16_t windows_driver[]={
        'W','I','N','S','P','O','O','L',0
    };
    assert(!memcmp(driver,windows_driver,sizeof windows_driver));
    assert(!memcmp(printer,f->expected_printer,sizeof f->expected_printer));
    assert(port == NULL);
    f->devmode_seen = devmode;
    ++f->created;
    return f->returns_null ? NULL : f;
}
static int32_t get_cap(void *context, void *dc, int32_t index)
{
    fake *f=context;
    assert(dc==f && f->probes<LEITZ_WIN2018_GDI_CAP_COUNT);
    f->indices[f->probes]=index;
    return f->returned[f->probes++];
}
static int32_t drop_dc(void *context, void *dc)
{
    fake *f=context;
    assert(dc == f);
    ++f->deleted;
    return f->last_destroy_value; /* Ignored by original. */
}
int main(void)
{
    static const int32_t expected[]={
        10,8,10,4,6,88,90,40,42,110,111,112,113
    };
    const int32_t *original=leitz_win2018_gdi_original_selectors();
    for(size_t i=0;i<LEITZ_WIN2018_GDI_CAP_COUNT;i++)
        assert(original[i]==expected[i]);
    static const uint16_t spool[]={'W','I','N','S','P','O','O','L',0};
    assert(!memcmp(leitz_win2018_gdi_original_driver(),spool,sizeof spool));

    fake f={0};f.expected_printer[0]='L';f.expected_printer[1]='I';
    int dm=41;
    f.returned[0]=301; /* VERTRES */
    f.returned[1]=601; /* HORZRES */
    f.returned[2]=301;
    f.last_destroy_value=-70;
    leitz_win2018_gdi_callbacks api={&f,make_dc,get_cap,drop_dc};
    leitz_win2018_row_control row={0};
    leitz_win2018_label_result result={0};
    leitz_win2018_gdi_caps_snapshot snap={0};
    leitz_win2018_label_input src={
        "CUSTOMSIZE",1,1,1,0,999
    };
    assert(leitz_win2018_gdi_initialize_label(&api,
        f.expected_printer,&dm,&src,&result,&row,&snap)==0);
    assert(f.created==1&&f.deleted==1&&f.probes==13);
    assert(f.devmode_seen==&dm);
    for(size_t i=0;i<LEITZ_WIN2018_GDI_CAP_COUNT;i++)
        assert(f.indices[i]==expected[i]&&snap.queried[i]==f.returned[i]);
    assert(snap.calls==13&&snap.raster_width==601&&snap.raster_height==301);
    assert(result.paper_width==601&&result.paper_height==301);
    assert(result.label_length==301&&result.is_custom_size==1);
    assert(row.physical_width==601&&row.label_height==301);

    /* Continuous manual stock uses the existing actual label_setup path. */
    f.probes=0;f.deleted=0;
    src.paper_name="Continuous70030001";src.configured_length=137;
    assert(leitz_win2018_gdi_initialize_label(&api,
        f.expected_printer,&dm,&src,&result,&row,&snap)==0);
    assert(result.is_die_cut==0&&result.label_length==137);
    assert(f.deleted==1&&f.probes==13);

    /* Layout orientation override was already reconstructed; this new
       adapter simply supplies real GDI geometry, not invented sizes. */
    f.probes=0;src.paper_name="CUSTOMSIZE";src.orientation=2;
    assert(leitz_win2018_gdi_initialize_label(&api,
        f.expected_printer,&dm,&src,&result,&row,&snap)==0);
    assert(result.paper_width==301&&result.paper_height==601);

    /* No context: never touch existing outputs or call GetDeviceCaps. */
    f.probes=0;f.deleted=0;f.returns_null=1;
    leitz_win2018_row_control before=row;
    leitz_win2018_label_result original_result=result;
    leitz_win2018_gdi_caps_snapshot original_snap=snap;
    assert(leitz_win2018_gdi_initialize_label(&api,
        f.expected_printer,&dm,&src,&result,&row,&snap)==-1);
    assert(f.probes==0&&f.deleted==0&&memcmp(&result,&original_result,sizeof result)==0);
    assert(!memcmp(&row,&before,sizeof row));
    assert(!memcmp(&snap,&original_snap,sizeof snap));
    f.returns_null=0;
    assert(leitz_win2018_gdi_initialize_label(NULL,
        f.expected_printer,&dm,&src,&result,&row,&snap)==-1);
    assert(leitz_win2018_gdi_initialize_label(&api,
        NULL,&dm,&src,&result,&row,&snap)==-1);
    assert(leitz_win2018_gdi_initialize_label(&api,
        f.expected_printer,&dm,&src,NULL,&row,&snap)==-1);
    assert(f.probes==0&&f.deleted==0);
    puts("LeitzRp original CreateDCW/GetDeviceCaps/DeleteDC label setup: PASS");
    return 0;
}
