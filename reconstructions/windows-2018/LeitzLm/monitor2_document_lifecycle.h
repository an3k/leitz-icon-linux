#ifndef LEITZ_WIN2018_MONITOR2_DOCUMENT_LIFECYCLE_H
#define LEITZ_WIN2018_MONITOR2_DOCUMENT_LIFECYCLE_H
#include <stddef.h>
#include <stdint.h>

/* Original LeitzLm.dll MONITOR2 callbacks:
 * EndDocPort slot 7: x86 RVA 0x13d0, x64 RVA 0x1600.
 * ClosePort slot 8:  x86 RVA 0x1040, x64 RVA 0x1100.
 * Portable callback/ownership model; not Windows MONITOR2 ABI.
 * Native per-port delegate offsets: x86 EndDoc +0x30, Close +0x34,
 * x64 EndDoc +0x60, Close +0x68; underlying handle +0x0c/+0x18.
 * Event handle is x86 +0x70, x64 +0xe0.
 */
#define LEITZ_LM_MONITOR_SETJOB_COMMAND 7u

typedef struct leitz_lm_document_job {
    struct leitz_lm_document_job *next;
    void *printer_handle;       /* native x86 job +8; x64 job +16 */
    uint32_t job_id;            /* native x86 job +12; x64 job +24 */
} leitz_lm_document_job;

typedef int32_t (*leitz_lm_end_document_fn)(void *, void *);
typedef int32_t (*leitz_lm_close_port_fn)(void *, void *);
typedef int32_t (*leitz_lm_set_event_fn)(void *, void *);
typedef int32_t (*leitz_lm_set_job_fn)(void *, void *, uint32_t,
                                       uint32_t, const void *, uint32_t);
typedef void (*leitz_lm_release_job_fn)(void *, leitz_lm_document_job *);
typedef void (*leitz_lm_release_port_fn)(void *);

typedef struct {
    void *context;
    void *underlying_port;
    void *document_event;
    leitz_lm_end_document_fn end_document;
    leitz_lm_close_port_fn close_port;
    leitz_lm_set_event_fn signal_event;
    leitz_lm_set_job_fn set_job;
    leitz_lm_release_job_fn release_job;
    leitz_lm_release_port_fn release_port;
    leitz_lm_document_job *jobs;
} leitz_lm_document_port;

/* Exact call order from both OEM DLLs:
 * SetEvent(event) -> native EndDocPort delegate -> for every job
 * SetJobW(printer, id, level=0, NULL, command=7) -> free every job.
 * Returns the EndDocPort delegate's BOOL unchanged, even if SetEvent/
 * SetJobW fail.  This portable version rejects invalid callbacks and
 * cyclic/wild lists only insofar as the caller honors list ownership;
 * no Win32 synchronization or memory access outside supplied nodes.
 * Actual native job-free traversal happens after all SetJobW calls.
 */
int32_t leitz_lm_monitor2_end_doc(leitz_lm_document_port *port);

/* If the native stored ClosePort callback is not NULL, call it once with
 * the underlying port, ignore its BOOL result, then release the native
 * port object. Return TRUE irrespective of the underlying result.
 * This C version refuses missing release_port instead of leaking memory;
 * caller must not access/reuse the released port afterwards.
 */
int32_t leitz_lm_monitor2_close_port(leitz_lm_document_port *port);
#endif
