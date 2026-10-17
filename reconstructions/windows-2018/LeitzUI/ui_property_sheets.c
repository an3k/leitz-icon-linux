#include "ui_property_sheets.h"
#include <stdint.h>
#include <string.h>

static void put16(uint8_t *p,uint16_t n) {
    p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);
}
static void put32(uint8_t *p,uint32_t n) {
    for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(n>>(8u*i));
}
static int layout_data(leitz_win2018_ui_layout layout,
                       leitz_win2018_ui_sheet_info *info) {
    if (layout!=LEITZ_UI_LAYOUT_X86 && layout!=LEITZ_UI_LAYOUT_X64)
        return -1;
    if(layout==LEITZ_UI_LAYOUT_X86) {
        info->option_item_bytes=0x34u;
        info->option_type_bytes=0x20u;
        info->option_parameter_bytes=0x18u;
        info->selection_offset=0x10u;
        info->option_item_zero_byte_offset=0x20u;
    } else {
        info->option_item_bytes=0x58u;
        info->option_type_bytes=0x30u;
        info->option_parameter_bytes=0x30u;
        info->selection_offset=0x18u;
        info->option_item_zero_byte_offset=0x34u;
    }
    return 0;
}

int leitz_win2018_ui_property_items(
    leitz_win2018_ui_layout layout, leitz_win2018_ui_sheet sheet,
    const uint16_t *initial_selection, uint8_t *items,
    size_t count, size_t capacity, size_t *required_count,
    leitz_win2018_ui_sheet_info *information)
{
    leitz_win2018_ui_sheet_info local={0};
    if(layout_data(layout,&local)!=0 ||
       (sheet!=LEITZ_UI_SHEET_DOCUMENT && sheet!=LEITZ_UI_SHEET_PRINTER) ||
       !required_count || !information) return -1;
    local.callback_type=sheet==LEITZ_UI_SHEET_DOCUMENT ?
        LEITZ_UI_CALLBACK_DOCUMENT : LEITZ_UI_CALLBACK_PRINTER;
    if(!items) {
        /* Original printer/doc method: if pOptItem is NULL, set count=1. */
        *required_count=1;
        *information=local;
        return 0;
    }
    if(count==0 || count>SIZE_MAX/local.option_item_bytes ||
       count*local.option_item_bytes>capacity) return -1;
    memset(items,0,count*local.option_item_bytes);
    for(size_t i=0;i<count;i++) {
        uint8_t *p=items+i*local.option_item_bytes;
        put16(p,(uint16_t)local.option_item_bytes);
        p[local.option_item_zero_byte_offset]=0;
    }
    items[2]=1;
    put32(items+4u,sheet==LEITZ_UI_SHEET_DOCUMENT ? 0x0cu : 0x04u);
    put32(items+local.selection_offset,
          initial_selection ? (uint32_t)*initial_selection : 0u);
    *information=local;
    *required_count=count; /* Original leaves supplied count untouched. */
    return 0;
}

int leitz_win2018_ui_property_type(
    leitz_win2018_ui_layout layout,
    uint8_t *type, size_t type_capacity,
    uint8_t *param, size_t param_capacity)
{
    leitz_win2018_ui_sheet_info info={0};
    if(layout_data(layout,&info)!=0 || !type || !param ||
       type_capacity<info.option_type_bytes ||
       param_capacity<info.option_parameter_bytes) return -1;
    memset(type,0,info.option_type_bytes);
    memset(param,0,info.option_parameter_bytes);
    put16(type,(uint16_t)info.option_type_bytes);
    put16(type+4u,1u); /* OEM cParams */
    put16(param,(uint16_t)info.option_parameter_bytes);
    type[2]=9; /* original documents: choice type at byte +2 */
    return 0;
}
