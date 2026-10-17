#include "oem_lifetime.h"
#include <limits.h>
#include <stddef.h>

int leitz_rp_lifetime_init(leitz_rp_oem_lifetime *obj,
                           leitz_rp_object_kind kind,
                           atomic_int *raster_global,
                           void *cached_host,
                           leitz_rp_release_host_fn release_host,
                           void *release_context)
{
    if (!obj || (kind != LEITZ_RP_OBJECT_FACTORY &&
                 kind != LEITZ_RP_OBJECT_RASTER) ||
        (kind == LEITZ_RP_OBJECT_RASTER && !raster_global) ||
        (cached_host && (kind != LEITZ_RP_OBJECT_RASTER || !release_host)))
        return -1;
    if (kind == LEITZ_RP_OBJECT_RASTER) {
        int old = atomic_load(raster_global);
        do {
            if (old == INT_MAX) return -1;
        } while (!atomic_compare_exchange_weak(raster_global, &old, old + 1));
    }
    atomic_init(&obj->reference_count, 1u);
    obj->kind = kind;
    obj->raster_global = kind == LEITZ_RP_OBJECT_RASTER ? raster_global : NULL;
    obj->cached_host = cached_host;
    obj->release_host = release_host;
    obj->release_context = release_context;
    return 0;
}

int leitz_rp_lifetime_addref(leitz_rp_oem_lifetime *obj,
                             unsigned *new_count)
{
    if (!obj || !new_count) return -1;
    unsigned old = atomic_load(&obj->reference_count);
    do {
        if (old == 0u || old == UINT_MAX) return -1;
    } while (!atomic_compare_exchange_weak(&obj->reference_count, &old, old + 1u));
    *new_count = old + 1u;
    return 0;
}

int leitz_rp_lifetime_release(leitz_rp_oem_lifetime *obj,
                              unsigned *new_count)
{
    if (!obj || !new_count) return -1;
    unsigned old = atomic_load(&obj->reference_count);
    do {
        if (old == 0u) return -1;
    } while (!atomic_compare_exchange_weak(&obj->reference_count, &old, old - 1u));
    *new_count = old - 1u;
    if (old != 1u) return 0;
    if (obj->kind == LEITZ_RP_OBJECT_RASTER) {
        if (obj->cached_host) {
            /* Guaranteed by init, unless caller mutates this state. */
            obj->release_host(obj->release_context, obj->cached_host);
            obj->cached_host = NULL;
        }
        (void)atomic_fetch_sub(obj->raster_global, 1);
    }
    return 0;
}
