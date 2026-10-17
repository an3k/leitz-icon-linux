#include "ui_property_sheets.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned word(const uint8_t *p){return (unsigned)p[0]+((unsigned)p[1]<<8u);}
static unsigned dword(const uint8_t *p){return word(p)+(word(p+2)<<16u);}
static void check(leitz_win2018_ui_layout arch,leitz_win2018_ui_sheet sheet,
                  size_t stride,size_t off,size_t zero,
                  size_t typesize,size_t paramsize,uint32_t flags)
{
    uint8_t items[3*88+3],type[50],param[50];
    leitz_win2018_ui_sheet_info info={0};
    size_t required=777;
    const uint16_t selected=0xbeefu;
    memset(items,0xa5,sizeof items);
    assert(leitz_win2018_ui_property_items(arch,sheet,&selected,NULL,0,0,&required,&info)==0);
    assert(required==1 && info.option_item_bytes==stride);
    assert(info.selection_offset==off && info.option_item_zero_byte_offset==zero);
    assert(info.option_type_bytes==typesize && info.option_parameter_bytes==paramsize);
    assert(info.callback_type == (sheet==LEITZ_UI_SHEET_DOCUMENT ? LEITZ_UI_CALLBACK_DOCUMENT : LEITZ_UI_CALLBACK_PRINTER));
    required=3;
    assert(leitz_win2018_ui_property_items(arch,sheet,&selected,items,3,3*stride,&required,&info)==0);
    assert(required==3);
    for(size_t i=0;i<3;i++) {
        assert(word(items+i*stride)==stride);
        assert(items[i*stride+zero]==0);
    }
    assert(items[2]==1 && dword(items+4)==flags && dword(items+off)==0xbeef);
    assert(items[stride+2]==0 && items[2*stride+2]==0);
    assert(items[3*stride]==0xa5 && items[3*stride+2]==0xa5);
    memset(type,0xa5,sizeof type);memset(param,0xa5,sizeof param);
    assert(leitz_win2018_ui_property_type(arch,type,typesize-1,param,paramsize)==-1);
    assert(type[0]==0xa5 && param[0]==0xa5);
    assert(leitz_win2018_ui_property_type(arch,type,typesize,param,paramsize-1)==-1);
    assert(type[0]==0xa5 && param[0]==0xa5);
    assert(leitz_win2018_ui_property_type(arch,type,typesize,param,paramsize)==0);
    assert(word(type)==typesize && type[2]==9 && word(type+4)==1);
    assert(word(param)==paramsize && type[typesize]==0xa5 && param[paramsize]==0xa5);
    for(size_t i=6;i<typesize;i++)assert(type[i]==0);
    for(size_t i=2;i<paramsize;i++)assert(param[i]==0);
}
int main(void)
{
    check(LEITZ_UI_LAYOUT_X86,LEITZ_UI_SHEET_DOCUMENT,0x34,0x10,0x20,0x20,0x18,0xc);
    check(LEITZ_UI_LAYOUT_X86,LEITZ_UI_SHEET_PRINTER,0x34,0x10,0x20,0x20,0x18,0x4);
    check(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,0x58,0x18,0x34,0x30,0x30,0xc);
    check(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_PRINTER,0x58,0x18,0x34,0x30,0x30,0x4);
    uint8_t data[88];memset(data,0x55,sizeof data);
    size_t count=44;
    leitz_win2018_ui_sheet_info info={.option_item_bytes=99};
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,0,sizeof data,&count,&info)==-1);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,1,87,&count,&info)==-1);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,SIZE_MAX,sizeof data,&count,&info)==-1);
    assert(leitz_win2018_ui_property_items((leitz_win2018_ui_layout)16,LEITZ_UI_SHEET_DOCUMENT,NULL,data,1,sizeof data,&count,&info)==-1);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,(leitz_win2018_ui_sheet)3,NULL,data,1,sizeof data,&count,&info)==-1);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,1,sizeof data,NULL,&info)==-1);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,1,sizeof data,&count,NULL)==-1);
    assert(count==44 && info.option_item_bytes==99);
    for(size_t i=0;i<sizeof data;i++)assert(data[i]==0x55);
    assert(leitz_win2018_ui_property_items(LEITZ_UI_LAYOUT_X64,LEITZ_UI_SHEET_DOCUMENT,NULL,data,1,sizeof data,&count,&info)==0);
    assert(dword(data+0x18)==0 && count==1);
    puts("Original Windows OEM UI property-sheet x86/x64 layouts and mode 1/2: PASS");
    return 0;
}
