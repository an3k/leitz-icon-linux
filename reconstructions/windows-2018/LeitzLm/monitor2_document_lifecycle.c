#include "monitor2_document_lifecycle.h"

int32_t leitz_lm_monitor2_end_doc(leitz_lm_document_port *port)
{
    if (!port || !port->signal_event || !port->end_document ||
        (port->jobs && (!port->set_job || !port->release_job)))
        return 0;

    /* x86 0x13d8 / x64 0x1612. The original ignores SetEvent's BOOL. */
    (void)port->signal_event(port->context, port->document_event);

    /* x86 0x13e1-0x13e7 / x64 0x161f-0x1623. */
    const int32_t result =
        port->end_document(port->context, port->underlying_port);

    /* x86 0x1400-0x141c / x64 0x1631-0x1662: continue after failures. */
    for (leitz_lm_document_job *item=port->jobs; item; item=item->next)
        (void)port->set_job(port->context,item->printer_handle,item->job_id,
                            0u,NULL,LEITZ_LM_MONITOR_SETJOB_COMMAND);

    /* x86 0x1b20 / x64 0x1e60 free each node, then clear head. */
    leitz_lm_document_job *item=port->jobs;
    port->jobs=NULL;
    while (item) {
        leitz_lm_document_job *next=item->next;
        port->release_job(port->context,item);
        item=next;
    }
    return result;
}

int32_t leitz_lm_monitor2_close_port(leitz_lm_document_port *port)
{
    if (!port || !port->release_port) return 0;
    /* x86 0x1047-0x1051 / x64 0x1106-0x1116. */
    if (port->close_port)
        (void)port->close_port(port->context,port->underlying_port);
    /* x86 0x105f / x64 0x1128: original port/object release routine
     * also unlinks associated spooler resources, not modeled here. */
    port->release_port(port->context);
    return 1;
}
