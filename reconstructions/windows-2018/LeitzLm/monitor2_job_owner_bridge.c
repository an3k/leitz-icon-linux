#include "monitor2_job_owner_bridge.h"

int leitz_lm_job_owner_views_match(const leitz_lm_job_owner_bridge *bridge)
{
    return bridge && bridge->owned && bridge->document &&
        bridge->owned->jobs == bridge->document->jobs;
}

int32_t leitz_lm_job_owner_start(leitz_lm_job_owner_bridge *bridge,
    const uint16_t *name, size_t capacity, uint32_t id, uint32_t level,
    const void *info, const leitz_lm_start_doc_callbacks *callbacks)
{
    if (!leitz_lm_job_owner_views_match(bridge)) return -1;
    const int32_t result=leitz_lm_monitor2_start_doc(bridge->document,
        name,capacity,id,level,info,callbacks);
    /* A failing *host StartDocPort* can still register a job in the OEM. */
    bridge->owned->jobs=bridge->document->jobs;
    return result;
}

int32_t leitz_lm_job_owner_end(leitz_lm_job_owner_bridge *bridge)
{
    if (!leitz_lm_job_owner_views_match(bridge)) return -1;
    const int32_t result=leitz_lm_monitor2_end_doc(bridge->document);
    /* EndDocPort consumes/frees the same list the owned destructor sees. */
    bridge->owned->jobs=bridge->document->jobs;
    return result;
}

int32_t leitz_lm_job_owner_close(leitz_lm_job_owner_bridge *bridge)
{
    if (!leitz_lm_job_owner_views_match(bridge) ||
        !bridge->document->release_port) return -1;
    /* Owned destructor must be the sole final job-list deallocator.
     * Clear the non-owning view before invoking this release callback. */
    leitz_lm_document_job *saved=bridge->document->jobs;
    bridge->document->jobs=NULL;
    const int32_t result=leitz_lm_monitor2_close_port(bridge->document);
    if (!result) {
        bridge->document->jobs=saved;
        return result;
    }
    bridge->owned=NULL;  /* Native port has been freed by release_port. */
    return result;
}
