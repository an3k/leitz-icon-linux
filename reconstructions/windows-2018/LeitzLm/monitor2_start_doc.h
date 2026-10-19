#ifndef LEITZ_WIN2018_MONITOR2_START_DOC_H
#define LEITZ_WIN2018_MONITOR2_START_DOC_H
#include "monitor2_document_lifecycle.h"
#include <stddef.h>
#include <stdint.h>

/* Original LeitzLm.dll MONITOR2 slot 4: StartDocPort, i386 RVA 0x12e0,
 * amd64 RVA 0x14b0. Portable model; not a Win32 MONITOR2 entry point.
 * This entry uses the existing leitz_lm_document_job and port model so
 * its registered jobs can be consumed by the existing EndDocPort model.
 */
#define LEITZ_LM_START_WAIT_MS UINT32_C(60000)
typedef int32_t (*leitz_lm_start_wait)(void *, void *, uint32_t);
typedef int32_t (*leitz_lm_start_reset)(void *, void *);
typedef leitz_lm_document_job *(*leitz_lm_start_alloc)(void *, const uint16_t *, size_t);
typedef int32_t (*leitz_lm_start_open_printer)(void *, const uint16_t *, void **);
typedef int32_t (*leitz_lm_start_delegate)(void *, void *, const uint16_t *, uint32_t, uint32_t, const void *);
typedef void (*leitz_lm_start_error)(void *, uint32_t);
typedef struct {
    leitz_lm_start_wait wait;
    leitz_lm_start_reset reset;
    leitz_lm_start_alloc allocate_job;
    leitz_lm_start_open_printer open_printer;
    leitz_lm_start_delegate start_document;
    leitz_lm_start_error set_error;
    /* On an unsuccessful OpenPrinterW, this safe model releases its
     * unlinked allocation. The original may leave it allocated. */
    leitz_lm_release_job_fn release_failed_job;
} leitz_lm_start_doc_callbacks;

/* Preconditions: non-NULL port, nonempty terminated printer name,
 * document info and mandatory callbacks. Original SetLastError(87) on
 * bad arguments; the portable model limits name scanning to
 * name_capacity_units to avoid wild reads and rejects overflow.
 * Steps: WaitForSingleObject(event, 60000), ResetEvent(event) (ignore
 * both results), allocate/copy UTF16 name, OpenPrinterW, record job id,
 * prepend job to port list, then call native StartDocPort host delegate.
 * If host callback fails, retain the job on the list until EndDocPort.
 * The result is the host delegate's BOOL. Neither an event result nor
 * a delayed SetJobW request changes the host return.
 */
int32_t leitz_lm_monitor2_start_doc(leitz_lm_document_port *port,
    const uint16_t *printer_name, size_t name_capacity_units,
    uint32_t job_id, uint32_t level, const void *document_info,
    const leitz_lm_start_doc_callbacks *callbacks);
#endif
