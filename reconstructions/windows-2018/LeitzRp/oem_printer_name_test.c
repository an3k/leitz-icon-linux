#include "oem_printer_name.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int calls,first_bool,second_bool,order_ok;
    uint32_t need,seen_size;
    void *expected_handle;
    const uint16_t *name;
    size_t name_units;
} query_case;

static int query(void *ctx, void *handle, uint32_t level,
                 uint8_t *buffer, uint32_t bytes, uint32_t *needed,
                 const uint16_t **name, size_t *units)
{
    query_case *q=ctx;
    assert(level==1 && handle==q->expected_handle && needed && name && units);
    ++q->calls;
    if(q->calls==1) {
        assert(buffer==NULL && bytes==0);
        *needed=q->need;
        return q->first_bool;
    }
    assert(q->calls==2 && buffer && bytes==q->need);
    q->seen_size=bytes;
    q->order_ok=1;
    *needed=q->need;
    *name=q->name;
    *units=q->name_units;
    return q->second_bool;
}

int main(void)
{
    static const uint16_t original_name[]={ 'L','e','i','t','z',' ','I','c','o','n',0 };
    uint16_t buffer[24];
    int fake_handle=51;
    query_case q={.first_bool=0,.second_bool=1,.need=112,
                  .expected_handle=&fake_handle,.name=original_name,
                  .name_units=10};
    size_t used=777;
    memset(buffer,0xa5,sizeof buffer);
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,&used)==0);
    assert(q.calls==2 && q.order_ok && q.seen_size==112);
    assert(used==10 && !memcmp(buffer,original_name,sizeof original_name));
    assert(buffer[11]==UINT16_C(0xa5a5));

    /* First BOOL result is ignored even when the first query failed. */
    q.calls=0;q.first_bool=1;q.second_bool=0;
    buffer[0]=0x7777;used=123;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,&used)==0);
    assert(q.calls==2 && buffer[0]==0x7777 && used==123);

    /* Zero required byte count suppresses the second GetPrinterW call. */
    q.calls=0;q.need=0;q.second_bool=1;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,&used)==0);
    assert(q.calls==1 && buffer[0]==0x7777 && used==123);

    /* Second call succeeds but does not provide a name pointer. */
    q.calls=0;q.need=32;q.name=NULL;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,&used)==0);
    assert(q.calls==2 && used==123 && buffer[0]==0x7777);

    q.calls=0;q.name=original_name;q.name_units=10;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,10,&used)==-1);
    assert(q.calls==2 && used==123 && buffer[0]==0x7777);
    q.calls=0;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,11,&used)==0);
    assert(used==10 && buffer[10]==0);

    q.calls=0;used=44;
    q.name_units=SIZE_MAX;
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,&used)==-1);
    assert(used==44);
    assert(leitz_rp_get_printer_name(NULL,&q,&fake_handle,buffer,24,&used)==-1);
    assert(leitz_rp_get_printer_name(query,&q,NULL,buffer,24,&used)==-1);
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,NULL,24,&used)==-1);
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,0,&used)==-1);
    assert(leitz_rp_get_printer_name(query,&q,&fake_handle,buffer,24,NULL)==-1);
    assert(q.calls==2);
    puts("Windows original GetPrinterW level-1 two-pass printer-name retrieval: PASS");
    return 0;
}
