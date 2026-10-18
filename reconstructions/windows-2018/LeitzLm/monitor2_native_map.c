#include "monitor2_native_map.h"

static const leitz_win2018_monitor2_entry original_table[LEITZ_WIN2018_MONITOR2_SLOTS] = {
    {0,0,LEITZ_LM_MONITOR_TABLE_SIZE},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0x11a0,0x12a0,LEITZ_LM_MONITOR_OPEN_PORT_EX},
    {0x12e0,0x14b0,LEITZ_LM_MONITOR_START_DOC},
    {0x1150,0x1220,LEITZ_LM_MONITOR_WRITE_PORT},
    {0x1080,0x1150,LEITZ_LM_MONITOR_READ_PORT},
    {0x13d0,0x1600,LEITZ_LM_MONITOR_END_DOC},
    {0x1040,0x1100,LEITZ_LM_MONITOR_CLOSE_PORT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0x1440,0x1000,LEITZ_LM_MONITOR_GET_PRINTER_DATA},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0x1000,0x10b0,LEITZ_LM_MONITOR_SHUTDOWN},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT},
    {0,0,LEITZ_LM_MONITOR_ABSENT}
};
const leitz_win2018_monitor2_entry *leitz_win2018_monitor2_slot(size_t index)
{
    return index < LEITZ_WIN2018_MONITOR2_SLOTS ? &original_table[index] : 0;
}
uint32_t leitz_win2018_monitor2_table_bytes(leitz_win2018_monitor2_arch arch)
{
    if (arch == LEITZ_LM_ARCH_X86) return 0x58u;
    if (arch == LEITZ_LM_ARCH_X64) return 0xb0u;
    return 0;
}
uint32_t leitz_win2018_monitor2_export_rva(leitz_win2018_monitor2_arch arch)
{
    if (arch == LEITZ_LM_ARCH_X86) return 0x1560u;
    if (arch == LEITZ_LM_ARCH_X64) return 0x16e0u;
    return 0;
}
int leitz_win2018_monitor2_initialize(leitz_win2018_monitor2_arch arch,
    void *registry, int allocation_succeeded,
    leitz_win2018_monitor2_instance *out,
    leitz_win2018_monitor2_error error, void *error_context)
{
    const uint32_t table_size=leitz_win2018_monitor2_table_bytes(arch);
    if (!table_size || !out) return -1;
    /* Native stores NULL to the caller's result handle before allocation. */
    out->registry_context=0;
    out->native_object_bytes=0;
    out->native_callback_table_bytes=0;
    out->native_callback_count=0;
    if (!allocation_succeeded) {
        if (error) error(error_context, 0u);
        return -1;
    }
    out->registry_context=registry;
    out->native_object_bytes=arch==LEITZ_LM_ARCH_X86 ? 12u : 24u;
    out->native_callback_table_bytes=table_size;
    out->native_callback_count=LEITZ_WIN2018_MONITOR2_SLOTS;
    return 0;
}
