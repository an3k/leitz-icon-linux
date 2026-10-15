#ifndef ORIGINAL_RASTER_NATIVE_PAGE_ENTRY_H
#define ORIGINAL_RASTER_NATIVE_PAGE_ENTRY_H
#include "original_read_page.h"
#include <stdint.h>

/* Authenticated macOS 5.1.465 raster2printer Mach-O ABI:
 * StartPage receives the descriptor pointer in rdi at 0x100005fbe;
 * the callee dereferences rdi at 0x100007399.
 * ProcessPage receives rdi=descriptor, rsi=page pixels, edx=page
 * index and ecx=reprint at 0x100005ff8..0x100006011.
 * EndPage receives a 0x10a8-byte descriptor by value on the stack
 * at 0x100006016..0x10000605e; 0x1000089bb accesses rbp+0x10.
 * These are actual function surfaces, not the internal callback ABI.
 * OEM private C++ data layout and other function ABIs remain partial.
 */
/* OEM _raster2printer call 0x100005f9e..0x100005fb9:
 * rdi=descriptor, rsi=RGB, rdx=monochrome, ecx=quality. */
void DitherPage(classic_oem_read_descriptor *descriptor,
                const uint8_t *rgb,uint8_t *monochrome,unsigned quality);
void StartPage(classic_oem_read_descriptor *descriptor);
int ProcessPage(classic_oem_read_descriptor *descriptor,
                const uint8_t *pixels,unsigned page_index,int reprint);
void EndPage(classic_oem_read_descriptor descriptor);
#endif
