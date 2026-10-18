#include "oem_printer_name.h"
#include "oem_gdi_caps.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int calls, dc_created, dc_deleted;
    uint32_t requested_size;
    const uint16_t *found_name;
    uint16_t requested_printer[24];
} host;
static int get_printer(void *context,void *h,uint32_t level,
    uint8_t *raw,uint32_t cb,uint32_t *required,
    const uint16_t **name,size_t *length)
{
    host *s=context;
    assert(h==context&&level==1);
    ++s->calls;
    *required=64;
    if (s->calls==1){assert(!raw&&!cb);return 0;}
    assert(s->calls==2&&raw&&cb==64);
    *name=s->found_name;
    *length=10;
    return 1;
}
static void *make_dc(void *ctx,const uint16_t *driver,
    const uint16_t *printer,const uint16_t *port,const void *devmode)
{
    host *s=ctx;
    static const uint16_t winspool[]={'W','I','N','S','P','O','O','L',0};
    assert(!memcmp(driver,winspool,sizeof winspool));
    assert(!memcmp(printer,s->requested_printer,22));
    assert(!port&&!devmode);
    ++s->dc_created;
    return s;
}
static int32_t get_cap(void *ctx,void *dc,int32_t selector)
{
    assert(dc==ctx);
    return selector==8 ? 480 : selector==10 ? 720 : 0;
}
static int32_t close_dc(void *ctx,void *dc)
{
    host *s=ctx;assert(ctx==dc);++s->dc_deleted;return 1;
}
int main(void)
{
    static const uint16_t name[]={'L','e','i','t','z',' ','I','c','o','n',0};
    host h={.found_name=name};
    size_t len=0;
    assert(leitz_rp_get_printer_name(get_printer,&h,&h,
        h.requested_printer,24,&len)==0);
    assert(h.calls==2&&len==10);
    leitz_win2018_gdi_callbacks api={&h,make_dc,get_cap,close_dc};
    leitz_win2018_label_input input={"CUSTOMSIZE",0,0,1,0,0};
    leitz_win2018_label_result result={0};
    leitz_win2018_row_control row={0};
    leitz_win2018_gdi_caps_snapshot snapshot={0};
    assert(leitz_win2018_gdi_initialize_label(&api,h.requested_printer,
       NULL,&input,&result,&row,&snapshot)==0);
    assert(h.dc_created==1&&h.dc_deleted==1);
    assert(result.paper_width==480&&result.paper_height==720&&
           result.label_length==720&&row.physical_width==480);
    assert(snapshot.calls==13&&snapshot.raster_width==480&&
           snapshot.raster_height==720);
    puts("Original GetPrinterW name -> GDI CreateDCW -> media geometry: PASS");
    return 0;
}
