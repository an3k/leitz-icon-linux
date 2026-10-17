#include "oem_row_dispatch.h"
#include <assert.h>
#include <stdio.h>

typedef struct { int calls; uint16_t observed; uint32_t length; int32_t result; } trace;
static int32_t alternate(void *context, leitz_win2018_raster_stream *s,
                         const uint8_t *pixels, uint32_t length)
{
    trace *t=context;
    assert(!length || pixels);
    t->calls++;
    t->observed=s->row->row_index;
    t->length=length;
    return t->result;
}
/* Existing already-tested stream implementation is isolated behind
 * its exported adapter. Production CI links the original sources. */
static int calls, value;
int32_t leitz_win2018_stream_counted_row(leitz_win2018_raster_stream *s,
                                         const uint8_t *p, uint32_t n)
{
    (void)p; (void)n;
    ++calls;
    value=s->row->row_index;
    return LEITZ_WIN2018_ROW_E_FAIL;
}
int main(void)
{
    leitz_win2018_row_control row={0};
    leitz_win2018_raster_stream s={0};
    s.row=&row;
    trace t={0};
    const uint8_t pixels[]={0xa5};
    assert(leitz_win2018_oem_dispatch_row(NULL,pixels,1,1,NULL,NULL)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(leitz_win2018_oem_dispatch_row(&s,NULL,1,1,NULL,NULL)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(leitz_win2018_oem_dispatch_row(&s,pixels,1,0,NULL,NULL)==
           LEITZ_WIN2018_OEM_ALT_PATH_MISSING);
    assert(row.row_index==0 && calls==0 && t.calls==0);
    assert(leitz_win2018_oem_dispatch_row(&s,pixels,1,1,NULL,NULL)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.row_index==1 && calls==1 && value==1);
    t.result=17;
    assert(leitz_win2018_oem_dispatch_row(&s,pixels,1,0,alternate,&t)==17);
    assert(row.row_index==2 && t.calls==1 && t.observed==2 && t.length==1);
    t.result=-71;
    assert(leitz_win2018_oem_dispatch_row(&s,NULL,0,0,alternate,&t)==-71);
    assert(row.row_index==3 && t.calls==2 && t.observed==3 && t.length==0);
    row.row_index=UINT16_MAX;
    assert(leitz_win2018_oem_dispatch_row(&s,pixels,1,1,NULL,NULL)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(row.row_index==0 && value==0 && calls==2);
    s.row=NULL;
    assert(leitz_win2018_oem_dispatch_row(&s,pixels,1,1,NULL,NULL)==
           LEITZ_WIN2018_ROW_E_FAIL);
    assert(calls==2);
    puts("Windows 2018 LeitzRp original raster method #15 row dispatch: PASS");
    return 0;
}
