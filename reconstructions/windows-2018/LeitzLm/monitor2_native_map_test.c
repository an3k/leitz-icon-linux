#include "monitor2_native_map.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static unsigned errors;
static uint32_t last_error=0x12345678u;
static void err(void *ctx, uint32_t code) {
    assert(ctx==&errors);
    ++errors;last_error=code;
}
int main(void) {
    static const uint32_t x86[LEITZ_WIN2018_MONITOR2_SLOTS]={
        0,0,0,0x11a0,0x12e0,0x1150,0x1080,0x13d0,0x1040,
        0,0,0,0,0x1440,0,0,0,0,0x1000,0,0,0
    };
    static const uint32_t x64[LEITZ_WIN2018_MONITOR2_SLOTS]={
        0,0,0,0x12a0,0x14b0,0x1220,0x1150,0x1600,0x1100,
        0,0,0,0,0x1000,0,0,0,0,0x10b0,0,0,0
    };
    static const leitz_win2018_monitor2_role roles[LEITZ_WIN2018_MONITOR2_SLOTS]={
        LEITZ_LM_MONITOR_TABLE_SIZE,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_OPEN_PORT_EX,
        LEITZ_LM_MONITOR_START_DOC,
        LEITZ_LM_MONITOR_WRITE_PORT,
        LEITZ_LM_MONITOR_READ_PORT,
        LEITZ_LM_MONITOR_END_DOC,
        LEITZ_LM_MONITOR_CLOSE_PORT,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_GET_PRINTER_DATA,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_SHUTDOWN,
        LEITZ_LM_MONITOR_ABSENT,LEITZ_LM_MONITOR_ABSENT,
        LEITZ_LM_MONITOR_ABSENT
    };
    for (size_t i=0;i<LEITZ_WIN2018_MONITOR2_SLOTS;i++) {
        const leitz_win2018_monitor2_entry *s=leitz_win2018_monitor2_slot(i);
        assert(s && s->x86_rva==x86[i] && s->x64_rva==x64[i]);
        assert(s->role==roles[i]);
    }
    assert(!leitz_win2018_monitor2_slot(22));
    assert(!leitz_win2018_monitor2_slot(SIZE_MAX));
    assert(leitz_win2018_monitor2_table_bytes(LEITZ_LM_ARCH_X86)==0x58);
    assert(leitz_win2018_monitor2_table_bytes(LEITZ_LM_ARCH_X64)==0xb0);
    assert(leitz_win2018_monitor2_table_bytes((leitz_win2018_monitor2_arch)16)==0);
    assert(leitz_win2018_monitor2_export_rva(LEITZ_LM_ARCH_X86)==0x1560);
    assert(leitz_win2018_monitor2_export_rva(LEITZ_LM_ARCH_X64)==0x16e0);
    assert(leitz_win2018_monitor2_export_rva((leitz_win2018_monitor2_arch)16)==0);
    int registry=42;
    leitz_win2018_monitor2_instance instance={0};
    assert(leitz_win2018_monitor2_initialize(LEITZ_LM_ARCH_X86,&registry,1,&instance,err,&errors)==0);
    assert(instance.registry_context==&registry);
    assert(instance.native_object_bytes==12 && instance.native_callback_table_bytes==0x58);
    assert(instance.native_callback_count==22 && errors==0);
    assert(leitz_win2018_monitor2_initialize(LEITZ_LM_ARCH_X64,&registry,1,&instance,err,&errors)==0);
    assert(instance.native_object_bytes==24 && instance.native_callback_table_bytes==0xb0);
    assert(leitz_win2018_monitor2_initialize(LEITZ_LM_ARCH_X86,&registry,0,&instance,err,&errors)==-1);
    assert(instance.registry_context==NULL && instance.native_object_bytes==0);
    assert(instance.native_callback_table_bytes==0 && instance.native_callback_count==0);
    assert(errors==1 && last_error==0);
    assert(leitz_win2018_monitor2_initialize((leitz_win2018_monitor2_arch)7,&registry,1,&instance,err,&errors)==-1);
    assert(errors==1);
    assert(leitz_win2018_monitor2_initialize(LEITZ_LM_ARCH_X64,&registry,1,NULL,err,&errors)==-1);
    puts("Original LeitzLm MONITOR2 22-slot x86/x64 table + InitializePrintMonitor2: PASS");
    return 0;
}
