#include "oem_paper_setting.h"
#include <stdlib.h>
#include <string.h>

int leitz_rp_oem_read_paper_setting(
    leitz_rp_driver_setting_query query, void *context,
    uint32_t max_allocation_bytes,
    uint16_t *name, size_t capacity, size_t *name_units,
    leitz_rp_paper_setting_info *info)
{
    if (!query || !name || !capacity || !name_units || !info)
        return -1;
    uint32_t needed=0,options=0;
    (void)query(context,"PaperSize",NULL,0,&needed,&options);
    leitz_rp_paper_setting_info next={0};
    next.first_needed_bytes=needed;
    next.last_needed_bytes=needed;
    next.options_returned=options;
    next.calls=1;
    if (!needed) {
        *name_units=0;
        *info=next;
        return 0;
    }
    if ((needed & 1u) || needed>max_allocation_bytes)
        return -1;
    uint8_t *data=malloc((size_t)needed);
    if(!data) return -1;
    memset(data,0xff,(size_t)needed);
    uint32_t second_needed=needed;
    (void)query(context,"PaperSize",data,needed,&second_needed,&options);
    next.calls=2;
    next.last_needed_bytes=second_needed;
    next.options_returned=options;
    const size_t max_units=(size_t)needed/2u;
    size_t n=0;
    while(n<max_units && (data[2u*n] || data[2u*n+1u]))++n;
    if (n==max_units || n>=capacity) {
        free(data);
        return -1;
    }
    for(size_t i=0;i<n;i++)
        name[i]=(uint16_t)((uint16_t)data[2u*i] | ((uint16_t)data[2u*i+1u]<<8u));
    name[n]=0;
    next.has_paper_name=1;
    *name_units=n;
    *info=next;
    free(data);
    return 0;
}
int leitz_rp_oem_paper_to_ascii(const uint16_t *name, size_t len,
                               char *dst, size_t cap)
{
    if(!name || !dst || len>=cap)return -1;
    for(size_t i=0;i<len;i++)
        if(!name[i] || name[i]>127u)return -1;
    for(size_t i=0;i<len;i++)dst[i]=(char)name[i];
    dst[len]=0;
    return 0;
}
