#include "monitor2_open_port.h"
#include <string.h>
int32_t leitz_lm_monitor2_open_port_ex(const uint16_t *name,
    void *monitor_handle,const uintptr_t *table,size_t entries,
    const leitz_lm_open_callbacks *cb,leitz_lm_open_port **out)
{
    if (!cb || !out || !table || entries<LEITZ_LM_OPEN_CALLBACK_SLOTS ||
        !table[2] || !table[4] || !table[5] || !table[6] || !table[8]) {
        if (cb && cb->error) cb->error(cb->context,LEITZ_LM_OPEN_ERROR_INVALID_MONITOR);
        return 0;
    }
    if (!name || !monitor_handle || !cb->enter_lock || !cb->leave_lock ||
        !cb->exists || !cb->allocate || !cb->release || !cb->host_open ||
        !cb->record)
        return 0;
    cb->enter_lock(cb->context);
    const int32_t found=cb->exists(cb->context,name);
    if (found) {
        if (cb->error) cb->error(cb->context,LEITZ_LM_OPEN_ERROR_ALREADY_EXISTS);
        cb->leave_lock(cb->context);
        return 0;
    }
    /* Manufacturer releases global lock before the host OpenPort call. */
    leitz_lm_open_port *port=cb->allocate(cb->context,name);
    cb->leave_lock(cb->context);
    if (!port) return 0;
    port->underlying_port=NULL;
    if (!cb->host_open(cb->context,monitor_handle,name,&port->underlying_port)) {
        cb->release(cb->context,port);
        return 0;
    }
    memcpy(port->host_table,table,sizeof port->host_table);
    port->monitor_context=monitor_handle;
    *out=port;
    cb->record(cb->context,port);
    return 1;
}
