#ifndef LEITZ_WIN2018_MONITOR2_JOB_OWNER_BRIDGE_H
#define LEITZ_WIN2018_MONITOR2_JOB_OWNER_BRIDGE_H
#include "monitor2_start_doc.h"
#include "monitor2_owned_cleanup.h"

/* Original i386 LeitzLm.dll StartDocPort RVA 0x138c writes the list
 * head at native port +8. amd64 RVA 0x1594 writes it at +16.
 * The original EndDocPort, ClosePort and Shutdown use the SAME list.
 * Previously reconstructed portable modules own two separate public
 * structs with independent `jobs` members. This bridge makes the
 * owned-port member authoritative, without changing those modules.
 * Caller serializes all operations (including Shutdown/ClosePort):
 * this is NOT a Win32 MONITOR2 ABI or a thread synchronization layer.
 */
typedef struct {
    leitz_lm_owned_port *owned;
    leitz_lm_document_port *document;
} leitz_lm_job_owner_bridge;

/* All operations return -1 for absent or desynchronized views without
 * taking ownership or invoking side-effect callbacks. In other cases,
 * preserve original delegate BOOL (0 / nonzero) from existing modules.
 */
int leitz_lm_job_owner_views_match(const leitz_lm_job_owner_bridge *bridge);
int32_t leitz_lm_job_owner_start(leitz_lm_job_owner_bridge *bridge,
    const uint16_t *printer_name, size_t name_capacity_units,
    uint32_t job_id, uint32_t level, const void *document_info,
    const leitz_lm_start_doc_callbacks *callbacks);
int32_t leitz_lm_job_owner_end(leitz_lm_job_owner_bridge *bridge);

/* ClosePort delegates to the existing native-owned destructor through
 * document->release_port. It will free owned and all remaining jobs.
 * On successful ClosePort, bridge->owned is invalidated and document
 * is left with jobs=NULL to prevent a dangling second owner.
 * The bridge struct and document view remain caller-owned; callers
 * must not reuse a closed bridge or dereference the freed port.
 */
int32_t leitz_lm_job_owner_close(leitz_lm_job_owner_bridge *bridge);
#endif
