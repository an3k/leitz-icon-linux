#ifndef LEITZ_WIN2018_RP_OEM_LIFETIME_H
#define LEITZ_WIN2018_RP_OEM_LIFETIME_H
#include <stdatomic.h>
#include <stdint.h>

/* Portable model of original LeitzRp.dll raster / class-factory
 * IUnknown AddRef + Release. NOT the native COM ABI or heap allocator.
 * Raster object's refcount: x86 +0x04, x64 +0x08.
 * Raster object's cached host: x86 +0x08, x64 +0x10.
 * Raster tracked global: x86 RVA 0x643c, x64 RVA 0x76f0.
 * The tracked global is distinct from LockServer and the TWO globals
 * consulted by DllCanUnloadNow. Do not conflate those counters.
 */
typedef enum {
    LEITZ_RP_OBJECT_FACTORY = 1,
    LEITZ_RP_OBJECT_RASTER = 2
} leitz_rp_object_kind;

typedef void (*leitz_rp_release_host_fn)(void *context, void *host);

typedef struct {
    atomic_uint reference_count;
    leitz_rp_object_kind kind;
    atomic_int *raster_global;
    void *cached_host;
    leitz_rp_release_host_fn release_host;
    void *release_context;
} leitz_rp_oem_lifetime;

/* Init is the native constructor's initial ref=1. Raster constructor
 * InterlockedIncrement's original separate tracked global.
 * For raster objects with cached_host set, release_host is required.
 * Counters and object must outlive the last Release; no real free().
 */
int leitz_rp_lifetime_init(leitz_rp_oem_lifetime *obj,
                           leitz_rp_object_kind kind,
                           atomic_int *raster_global,
                           void *cached_host,
                           leitz_rp_release_host_fn release_host,
                           void *release_context);

/* Both return -1 on invalid state/NULL/count overflow or underflow.
 * On success, write the refcount AFTER atomic increment/decrement.
 * Release of the final reference calls cached IUnknown::Release once,
 * clears it, and decrements the distinct raster-global counter once.
 * A portable caller retains/frees the C object itself. These calls
 * assume the caller retains ownership while invoking them, as in COM.
 */
int leitz_rp_lifetime_addref(leitz_rp_oem_lifetime *obj,
                             unsigned *new_count);
int leitz_rp_lifetime_release(leitz_rp_oem_lifetime *obj,
                              unsigned *new_count);
#endif
