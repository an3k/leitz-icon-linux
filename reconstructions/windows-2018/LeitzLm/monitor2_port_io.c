#include "monitor2_port_io.h"

int32_t leitz_win2018_monitor2_write(leitz_win2018_monitor2_port *port,
    const uint8_t *bytes, uint32_t requested, uint32_t *transferred)
{
    if (!port) return 0;
    if (!port->write || !transferred || (requested && !bytes)) {
        if (port->error) port->error(port->context, 87u);
        return 0;
    }
    return port->write(port->context, port->underlying_handle,
                       bytes, requested, transferred);
}
int32_t leitz_win2018_monitor2_read(leitz_win2018_monitor2_port *port,
    uint8_t *bytes, uint32_t requested, uint32_t *transferred)
{
    if (!port || !port->read || !transferred || (requested && !bytes))
        return 0;
    return port->read(port->context, port->underlying_handle,
                      bytes, requested, transferred);
}
