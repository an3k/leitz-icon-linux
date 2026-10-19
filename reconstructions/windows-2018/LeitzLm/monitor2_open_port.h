#ifndef LEITZ_WIN2018_MONITOR2_OPEN_PORT_H
#define LEITZ_WIN2018_MONITOR2_OPEN_PORT_H
#include <stddef.h>
#include <stdint.h>
#define LEITZ_LM_OPEN_CALLBACK_SLOTS 22u
#define LEITZ_LM_OPEN_ERROR_ALREADY_EXISTS 170u
#define LEITZ_LM_OPEN_ERROR_INVALID_MONITOR 3007u
/* LeitzLm.dll OpenPortEx x86 RVA 0x11a0 / x64 RVA 0x12a0.
 * Native MONITOR2 host-table required pointer indexes 2,4,5,6,8;
 * x86 offsets 8,16,20,24,32; x64 16,32,40,48,64.
 * Host table is normalized into uintptr_t cells, not a native ABI.
 */
typedef struct leitz_lm_open_port {
    void *underlying_port;
    void *monitor_context;
    uintptr_t host_table[LEITZ_LM_OPEN_CALLBACK_SLOTS];
} leitz_lm_open_port;
typedef void (*leitz_lm_open_lock_fn)(void *);
typedef int32_t (*leitz_lm_open_exists_fn)(void *,const uint16_t *);
typedef leitz_lm_open_port *(*leitz_lm_open_allocate_fn)(void *,const uint16_t *);
typedef void (*leitz_lm_open_release_fn)(void *,leitz_lm_open_port *);
typedef int32_t (*leitz_lm_open_delegate_fn)(void *,void *,const uint16_t *,void **);
typedef void (*leitz_lm_open_error_fn)(void *,uint32_t);
typedef void (*leitz_lm_open_record_fn)(void *,leitz_lm_open_port *);
typedef struct {
    void *context;
    leitz_lm_open_lock_fn enter_lock,leave_lock;
    leitz_lm_open_exists_fn exists;
    leitz_lm_open_allocate_fn allocate;
    leitz_lm_open_release_fn release;
    leitz_lm_open_delegate_fn host_open;
    leitz_lm_open_error_fn error;
    leitz_lm_open_record_fn record;
} leitz_lm_open_callbacks;
/* The existing OEM allocation routine already registers the new port
 * under lock. The caller's allocate callback owns that registration;
 * release removes it on host-open failure. Only successful host-open
 * copies the supplied host table and publishes the resulting pointer.
 * OEM errors: invalid host callback table -> SetLastError(3007),
 * duplicate named port -> SetLastError(170). Missing callback pointers
 * are safely rejected rather than dereferenced as the original could.
 * out remains unchanged on every failure.
 */
int32_t leitz_lm_monitor2_open_port_ex(const uint16_t *port_name,
    void *monitor_handle,const uintptr_t *original_host_table,size_t entries,
    const leitz_lm_open_callbacks *callbacks,leitz_lm_open_port **out);
#endif
