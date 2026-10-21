#include "monitor2_port_registry.h"
#include <limits.h>
#include <string.h>

_Static_assert(sizeof(leitz_lm_native_port) <= LEITZ_LM_X86_NATIVE_PORT_BYTES,
               "portable port record exceeds smallest original allocation");

leitz_lm_owned_port *leitz_lm_native_find_port(
    const leitz_lm_owned_registry *registry, const uint16_t *name,
    const leitz_lm_registry_services *services)
{
    if (!registry || !name || !*name || !services || !services->compare_ci)
        return NULL;
    for (leitz_lm_owned_port *p = registry->head; p; p = p->next) {
        if (p->name_allocation &&
            services->compare_ci(services->context, name,
                (const uint16_t *)p->name_allocation) == 0)
            return p;
    }
    return NULL;
}

leitz_lm_native_port *leitz_lm_native_create_port(
    leitz_lm_owned_registry *registry, const uint16_t *name,
    size_t name_capacity_units, leitz_win2018_monitor2_arch arch,
    const leitz_lm_registry_services *services)
{
    if (!registry || !name || !name_capacity_units || !services ||
        !services->allocate || !services->deallocate ||
        !services->create_event ||
        (arch != LEITZ_LM_ARCH_X86 && arch != LEITZ_LM_ARCH_X64))
        return NULL;
    size_t n = 0;
    while (n < name_capacity_units && name[n]) ++n;
    if (n == name_capacity_units || !n || n > (SIZE_MAX / 2u) - 1u)
        return NULL;
    const size_t native_bytes = arch == LEITZ_LM_ARCH_X86 ?
        LEITZ_LM_X86_NATIVE_PORT_BYTES : LEITZ_LM_X64_NATIVE_PORT_BYTES;
    leitz_lm_native_port *p = services->allocate(services->context, native_bytes);
    if (!p) return NULL;
    /* Original calls memset(raw port, 0, native_bytes) after GlobalAlloc. */
    memset(p, 0, native_bytes);
    const size_t name_bytes = (n + 1u) * sizeof(uint16_t);
    void *copied = services->allocate(services->context, name_bytes);
    if (!copied) {
        services->deallocate(services->context, p);
        return NULL;
    }
    memcpy(copied, name, name_bytes);
    p->owned.name_allocation = copied;
    p->allocation_bytes = (uint32_t)native_bytes;
    p->owned.next = registry->head;
    registry->head = &p->owned;
    /* Original CreateEventW(NULL, TRUE, TRUE, NULL) is after linking. */
    p->owned.event_handle = services->create_event(services->context, 1, 1);
    return p;
}
