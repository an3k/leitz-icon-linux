#ifndef LEITZ_WIN2018_MONITOR2_NATIVE_MAP_H
#define LEITZ_WIN2018_MONITOR2_NATIVE_MAP_H
#include <stddef.h>
#include <stdint.h>

/* Original Windows 2018 LeitzLm.dll InitializePrintMonitor2 export:
 * x86 RVA 0x1560; x64 RVA 0x16e0.
 * Both return a pointer to a 22-entry MONITOR2 callback table; the
 * first entry contains the BYTE SIZE 0x58 or 0xb0, not a function.
 * These are original RVA measurements, NOT executable function pointers.
 */
#define LEITZ_WIN2018_MONITOR2_SLOTS 22u
typedef enum {
    LEITZ_LM_MONITOR_TABLE_SIZE = 0,
    LEITZ_LM_MONITOR_ABSENT,
    LEITZ_LM_MONITOR_OPEN_PORT_EX,
    LEITZ_LM_MONITOR_START_DOC,
    LEITZ_LM_MONITOR_WRITE_PORT,
    LEITZ_LM_MONITOR_READ_PORT,
    LEITZ_LM_MONITOR_END_DOC,
    LEITZ_LM_MONITOR_CLOSE_PORT,
    LEITZ_LM_MONITOR_GET_PRINTER_DATA,
    LEITZ_LM_MONITOR_SHUTDOWN
} leitz_win2018_monitor2_role;

typedef enum {
    LEITZ_LM_ARCH_X86 = 32,
    LEITZ_LM_ARCH_X64 = 64
} leitz_win2018_monitor2_arch;

typedef struct {
    uint32_t x86_rva;
    uint32_t x64_rva;
    leitz_win2018_monitor2_role role;
} leitz_win2018_monitor2_entry;

const leitz_win2018_monitor2_entry *leitz_win2018_monitor2_slot(size_t index);
uint32_t leitz_win2018_monitor2_table_bytes(leitz_win2018_monitor2_arch arch);
uint32_t leitz_win2018_monitor2_export_rva(leitz_win2018_monitor2_arch arch);

/* Native initialization allocates 12 bytes (x86) / 24 bytes (x64),
 * initializes the caller's output handle to NULL before allocation,
 * saves registry/context at +4/+8 and clears +8/+16 on success.
 * If allocation fails, native SetLastError(0) is called (not ERROR 8).
 * Represent allocation as an externally supplied outcome to avoid
 * inventing OS allocator/heap lifetime or a native C++ pointer.
 */
typedef void (*leitz_win2018_monitor2_error)(void *, uint32_t);
typedef struct {
    void *registry_context;
    uint32_t native_object_bytes;
    uint32_t native_callback_table_bytes;
    uint32_t native_callback_count;
} leitz_win2018_monitor2_instance;
int leitz_win2018_monitor2_initialize(leitz_win2018_monitor2_arch arch,
    void *registry_context, int allocation_succeeded,
    leitz_win2018_monitor2_instance *instance,
    leitz_win2018_monitor2_error report_error, void *error_context);
#endif
