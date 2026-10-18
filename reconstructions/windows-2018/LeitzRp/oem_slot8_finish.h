#ifndef LEITZ_WIN2018_RP_OEM_SLOT8_FINISH_H
#define LEITZ_WIN2018_RP_OEM_SLOT8_FINISH_H
#include "oem_write_data.h"
#include <stdint.h>

/* Original LeitzRp.dll raster vtable slot 8, x86 RVA 0x14e0,
 * x64 RVA 0x14f0. Its role is not inferred from COM slot number.
 * The original reads WORD at +0x0c of a separately supplied medium,
 * compares accumulated WriteData bytes against 5*WORD+16, and only
 * when equal calls the write helper with the single byte ESC (0x1b).
 * It then frees a pointer stored in an allocation and the allocation.
 * Both free calls are externalized; not a native Windows COM/heap ABI.
 */
typedef void (*leitz_rp_oem_free_callback)(void *context, void *resource);

typedef struct {
    uint16_t medium_word_at_0c;
    void *allocated_block;
    void *nested_allocated_resource;
    leitz_rp_oem_free_callback free_resource;
    void *free_context;
} leitz_rp_oem_slot8_arguments;

/* Returns S_OK regardless of WriteData result, as original slot 8 does.
 * With invalid state/callbacks, returns E_FAIL safely with no changes.
 * The two cleanup callbacks execute in the original nested, outer order.
 * This caller-owned adapter does not itself call free(3) or clear the
 * pointers; it does not own their lifetime or track repeated invocation.
 */
int32_t leitz_rp_oem_slot8_finish(leitz_rp_oem_write_state *writer,
                                  const leitz_rp_oem_slot8_arguments *args);
#endif
