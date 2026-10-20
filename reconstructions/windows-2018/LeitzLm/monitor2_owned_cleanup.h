#ifndef LEITZ_WIN2018_MONITOR2_OWNED_CLEANUP_H
#define LEITZ_WIN2018_MONITOR2_OWNED_CLEANUP_H
#include "monitor2_document_lifecycle.h"

/* Original LeitzLm.dll shared port destruction helper, i386 RVA 0x1a60,
 * amd64 RVA 0x1d30. This owns a different object than the host
 * MONITOR2 ClosePort callback. Native offsets in x86 / x64:
 * global next +4/+8, job list +8/+16, printer handle +16/+32,
 * event handle +0x70/+0xe0, name pointer +0/+0.
 * Portable owner/list model; NOT a native Win32 struct or ABI.
 */
typedef struct leitz_lm_owned_port {
    void *name_allocation;
    struct leitz_lm_owned_port *next;
    leitz_lm_document_job *jobs;
    void *printer_for_delete;
    void *event_handle;
} leitz_lm_owned_port;
typedef struct {
    leitz_lm_owned_port *head;
    void *monitor_allocation;
} leitz_lm_owned_registry;

typedef int32_t (*leitz_lm_owned_handle_fn)(void *, void *);
typedef void (*leitz_lm_owned_lock_fn)(void *);
typedef void (*leitz_lm_owned_free_fn)(void *, void *);
typedef struct {
    leitz_lm_owned_handle_fn delete_printer;
    leitz_lm_owned_handle_fn close_event;
    leitz_lm_owned_lock_fn enter_job_lock;
    leitz_lm_owned_lock_fn leave_job_lock;
    leitz_lm_owned_free_fn free_global;
} leitz_lm_owned_callbacks;

/* Original shared helper unlinks the port from global registry, calls
 * WINSPOOL.DRV!DeletePrinter for a non-NULL printer handle (NOT
 * ClosePrinter), calls CloseHandle on event if non-NULL, frees all
 * job nodes within the original critical section, then GlobalFree's
 * port-name allocation and port itself. Result of DeletePrinter and
 * CloseHandle is ignored. The callback must own any destructive action:
 * nothing in this portable model calls DeletePrinter directly.
 * Returns 0 only if all required callbacks are available.
 * No callback error can stop OEM cleanup once begun.
 */
int leitz_lm_release_owned_port(leitz_lm_owned_registry *registry,
    leitz_lm_owned_port *port, const leitz_lm_owned_callbacks *callbacks,
    void *context);

/* Original LeitzLm MONITOR2 Shutdown, i386 RVA 0x1000, amd64 RVA 0x10b0:
 * traverse the monitor's current port list, calling the same helper,
 * then GlobalFree(monitor). It deliberately does NOT call the host
 * ClosePort delegate: this differs from normal ClosePort.
 * The caller must own the acyclic registry and keep callbacks valid.
 */
int leitz_lm_shutdown_owned_monitor(leitz_lm_owned_registry *registry,
    const leitz_lm_owned_callbacks *callbacks, void *context);
#endif
