#ifndef LEITZ_WIN2018_MONITOR2_PORT_REGISTRY_H
#define LEITZ_WIN2018_MONITOR2_PORT_REGISTRY_H
#include "monitor2_native_map.h"
#include "monitor2_owned_cleanup.h"
#include <stddef.h>
#include <stdint.h>

/* Original LeitzLm.dll helpers: i386 find RVA 0x1ae0, allocate RVA
 * 0x19b0; amd64 find RVA 0x1e00, allocate RVA 0x1c40.
 * Portable storage; the owned field is first for use with the already
 * recovered MONITOR2 port release helper.  No native Win32 ABI here. */
#define LEITZ_LM_X86_NATIVE_PORT_BYTES 0x478u
#define LEITZ_LM_X64_NATIVE_PORT_BYTES 0x4f0u
#define LEITZ_LM_NATIVE_SCRATCH_BYTES 0x400u

typedef struct {
    leitz_lm_owned_port owned;
    uint8_t port_scratch[LEITZ_LM_NATIVE_SCRATCH_BYTES];
    uint32_t allocation_bytes;
    uint8_t startup_flag;
} leitz_lm_native_port;

typedef int32_t (*leitz_lm_name_compare_ci)(void *, const uint16_t *, const uint16_t *);
typedef void *(*leitz_lm_native_alloc)(void *, size_t);
typedef void (*leitz_lm_native_free)(void *, void *);
typedef void *(*leitz_lm_create_manual_event)(void *, int manual_reset, int initially_signaled);
typedef struct {
    void *context;
    leitz_lm_name_compare_ci compare_ci;
    leitz_lm_native_alloc allocate;
    leitz_lm_native_free deallocate;
    leitz_lm_create_manual_event create_event;
} leitz_lm_registry_services;

/* OEM lstrcmpiW: zero means case-insensitive equality. Delegate
 * comparison so we do not pretend ASCII folding reproduces Windows
 * locale-aware Unicode comparisons. NULL / empty names are rejected.
 * The registry's critical section is held by its OpenPortEx caller.
 */
leitz_lm_owned_port *leitz_lm_native_find_port(
    const leitz_lm_owned_registry *registry, const uint16_t *name,
    const leitz_lm_registry_services *services);

/* Allocates native 0x478/0x4f0 zeroed bytes, creates a separate
 * NUL-terminated UTF-16 name allocation, prepends its owned node to
 * the registry, then creates manual-reset, initially-signaled event.
 * An event-creation failure is NOT fatal in the original; the event
 * field remains NULL. Missing callbacks/buffer overflow are rejected.
 * If name allocation fails the portable model safely frees the new
 * unlinked port (OEM has a hazardous null-dereference on that path).
 * Never call this helper on a duplicate: the OEM caller first uses
 * leitz_lm_native_find_port while holding its global lock.
 */
leitz_lm_native_port *leitz_lm_native_create_port(
    leitz_lm_owned_registry *registry, const uint16_t *name,
    size_t name_capacity_units, leitz_win2018_monitor2_arch arch,
    const leitz_lm_registry_services *services);
#endif
