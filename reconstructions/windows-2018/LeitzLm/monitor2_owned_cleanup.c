#include "monitor2_owned_cleanup.h"
#include <stddef.h>

static int valid(const leitz_lm_owned_callbacks *c)
{
    return c && c->delete_printer && c->close_event &&
        c->enter_job_lock && c->leave_job_lock && c->free_global;
}
int leitz_lm_release_owned_port(leitz_lm_owned_registry *registry,
    leitz_lm_owned_port *port, const leitz_lm_owned_callbacks *c,
    void *context)
{
    if (!registry || !port || !valid(c)) return -1;
    /* x86 0x1a63..0x1a95, x64 0x1d3a..0x1d7b: unlink first. */
    leitz_lm_owned_port **link = &registry->head;
    while (*link && *link != port) link = &(*link)->next;
    if (*link == port) *link = port->next;

    /* x86 0x1a95/0x1aa2, x64 0x1d7b/0x1d89.  The actual imported
     * function is DeletePrinter, so callers must decide what, if any,
     * real operation is appropriate; the callback is mandatory. */
    if (port->printer_for_delete)
        (void)c->delete_printer(context, port->printer_for_delete);
    if (port->event_handle)
        (void)c->close_event(context, port->event_handle);

    c->enter_job_lock(context);
    leitz_lm_document_job *item = port->jobs;
    port->jobs = NULL;
    while (item) {
        leitz_lm_document_job *next = item->next;
        c->free_global(context, item);
        item = next;
    }
    c->leave_job_lock(context);
    c->free_global(context, port->name_allocation);
    c->free_global(context, port);
    return 0;
}
int leitz_lm_shutdown_owned_monitor(leitz_lm_owned_registry *registry,
    const leitz_lm_owned_callbacks *c, void *context)
{
    if (!registry || !valid(c) || !registry->monitor_allocation) return -1;
    /* Original ShutDown saves next before the helper frees current. */
    while (registry->head) {
        leitz_lm_owned_port *current = registry->head;
        if (leitz_lm_release_owned_port(registry, current, c, context))
            return -1;
    }
    void *monitor = registry->monitor_allocation;
    registry->monitor_allocation = NULL;
    c->free_global(context, monitor);
    return 0;
}
