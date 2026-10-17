#include "oem_method_map.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* Independent expected vtable values from original PE binaries. */
    static const uint32_t x86[] = {
        0x1220,0x1070,0x1290,0x1ed0,0x15d0,0x1330,0x1340,
        0x1370,0x14e0,0x1170,0x1f80,0x1e80,0x15f0,0x1fc0,
        0x1f70,0x1ff0,0x1fd0,0x2b00,0x2b10,0x1fc0,0x1fe0,
        0x1fc0,0x1fe0,0x1fc0,0x1fe0,0x2b20
    };
    static const uint32_t x64[] = {
        0x1230,0x1060,0x1290,0x1f20,0x15f0,0x1310,0x1320,
        0x1350,0x14f0,0x1160,0x1ff0,0x1f00,0x1600,0x1fe0,
        0x1fe0,0x2040,0x1fe0,0x1fe0,0x1fe0,0x1fe0,0x1fe0,
        0x1fe0,0x1fe0,0x1fe0,0x1fe0,0x1fe0
    };
    _Static_assert(sizeof x86 / sizeof x86[0] == LEITZ_RP_OEM_METHOD_COUNT, "x86 vtable size");
    _Static_assert(sizeof x64 / sizeof x64[0] == LEITZ_RP_OEM_METHOD_COUNT, "x64 vtable size");
    unsigned notimpl = 0;
    uint32_t value = 0x12345678u;
    for (size_t i=0; i<LEITZ_RP_OEM_METHOD_COUNT; ++i) {
        const leitz_rp_method_entry *m = leitz_rp_oem_method_at(i);
        assert(m && m->x86_rva == x86[i] && m->x64_rva == x64[i]);
        if (i==5) {
            assert(m->kind==LEITZ_RP_METHOD_S_OK);
            assert(leitz_rp_oem_direct_return(i,&value)==0 && value==0);
        } else if (i==9) {
            assert(m->kind==LEITZ_RP_METHOD_ONE);
            assert(leitz_rp_oem_direct_return(i,&value)==0 && value==1);
        } else if (i==11) {
            assert(m->kind==LEITZ_RP_METHOD_FILTER_NAME);
            value=0x12345678u;
            assert(leitz_rp_oem_direct_return(i,&value)==-1 && value==0x12345678u);
        } else if (i==13 || i==14 || i>=16) {
            assert(m->kind==LEITZ_RP_METHOD_NOTIMPL);
            assert(leitz_rp_oem_direct_return(i,&value)==0 && value==LEITZ_RP_OEM_E_NOTIMPL);
            ++notimpl;
        } else {
            assert(m->kind==LEITZ_RP_METHOD_NATIVE);
            value=0x12345678u;
            assert(leitz_rp_oem_direct_return(i,&value)==-1 && value==0x12345678u);
        }
    }
    assert(notimpl==12);
    assert(leitz_rp_oem_filtergraphics_comparison("FilterGraphics")==0);
    assert(leitz_rp_oem_filtergraphics_comparison("filtergraphics")==1);
    assert(leitz_rp_oem_filtergraphics_comparison("FilterGraphics ")==1);
    assert(leitz_rp_oem_filtergraphics_comparison("OtherMethod")==1);
    assert(leitz_rp_oem_filtergraphics_comparison("")==1);
    assert(leitz_rp_oem_filtergraphics_comparison(NULL)==-1);
    value=0x12345678u;
    assert(leitz_rp_oem_direct_return(LEITZ_RP_OEM_METHOD_COUNT,&value)==-1 && value==0x12345678u);
    assert(leitz_rp_oem_direct_return(5,NULL)==-1);
    assert(leitz_rp_oem_method_at(26)==NULL);
    assert(leitz_rp_oem_method_at(SIZE_MAX)==NULL);
    puts("Windows OEM 26-slot x86/x64 LeitzRp vtables and FilterGraphics selector: PASS");
    return 0;
}
