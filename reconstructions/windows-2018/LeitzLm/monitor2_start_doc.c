#include "monitor2_start_doc.h"
#include <stdint.h>

int32_t leitz_lm_monitor2_start_doc(leitz_lm_document_port *port,
    const uint16_t *printer_name, size_t name_capacity_units,
    uint32_t job_id, uint32_t level, const void *document_info,
    const leitz_lm_start_doc_callbacks *callbacks)
{
    if (!port || !printer_name || !name_capacity_units || !document_info ||
        !callbacks || !callbacks->wait || !callbacks->reset ||
        !callbacks->allocate_job || !callbacks->open_printer ||
        !callbacks->start_document || !callbacks->release_failed_job) {
        if (port && callbacks && callbacks->set_error)
            callbacks->set_error(port->context,87u);
        return 0;
    }
    size_t length=0;
    while (length<name_capacity_units && printer_name[length])
        ++length;
    if (!length || length==name_capacity_units ||
        length > (UINT32_MAX-0x22u)/2u) {
        if (callbacks->set_error) callbacks->set_error(port->context,87u);
        return 0;
    }
    /* Native WaitForSingleObject/ResetEvent return values are ignored. */
    (void)callbacks->wait(port->context,port->document_event,LEITZ_LM_START_WAIT_MS);
    (void)callbacks->reset(port->context,port->document_event);
    leitz_lm_document_job *job = callbacks->allocate_job(
        port->context,printer_name,length);
    if (!job) return 0;
    job->printer_handle=NULL;
    job->job_id=job_id;
    job->next=NULL;
    if (!callbacks->open_printer(port->context,printer_name,&job->printer_handle)) {
        callbacks->release_failed_job(port->context,job);
        return 0;
    }
    job->next=port->jobs;
    port->jobs=job;
    return callbacks->start_document(port->context,port->underlying_port,
                                      printer_name,job_id,level,document_info);
}
