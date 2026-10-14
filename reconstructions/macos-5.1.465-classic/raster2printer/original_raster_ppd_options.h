#ifndef ORIGINAL_RASTER_PPD_OPTIONS_H
#define ORIGINAL_RASTER_PPD_OPTIONS_H
#include <stddef.h>
#include <stdint.h>

/* Classic macOS raster2printer x86_64 OEM descriptor: 0x10a8 bytes.
 * These option-specific offsets were established from manufacturer assembly
 * 0x1000045a0..0x100005412, not inferred from Linux layout. The caller
 * owns a real descriptor or an isolated 0x10a8-byte test buffer.
 */
enum {
  OEM_DESCRIPTOR_SIZE=0x10a8,
  OEM_PRINT_THROUGH_PRN_OFFSET=0x08,
  OEM_CUT_EVERY_LABEL_OFFSET=0x10,
  OEM_HIGH_RESOLUTION_OFFSET=0x14,
  OEM_DEBUG_MODE_OFFSET=0x18,
  OEM_OPTIMIZATION_OFFSET=0x40,
  OEM_PRINT_QUALITY_OFFSET=0x4c
};

typedef struct {
    void *context;
    /* Invokes OEM ppdFindMarkedChoice(ppd, key); returns pointer to
     * inline ppd_choice_t.choice or NULL if the choice is not marked. */
    const char *(*marked_choice)(void *, const char *key);
} classic_original_ppd_options_io;

/* The original memset(0,0x10a8) and optimizationLevel=1 initialization.
 * Use only BEFORE populating the original descriptor with CUPS state. */
void classic_original_raster_initialize_options(unsigned char *descriptor);

/* Recovered original resolution/quality/debug/prn/optimization/cut choices.
 * Native CUPS calls and original option list parsing live outside this
 * fragment; no additional macOS options may be inferred from Linux code. */
void classic_original_raster_select_options(
    unsigned char *descriptor,const classic_original_ppd_options_io *io);

/* OEM diagnostic emitted after cupsFreeOptions() and ppdClose(), at line 249.
 * Does not call either of those functions; original order is caller-owned. */
void classic_original_raster_report_options(
    const unsigned char *descriptor,const classic_original_ppd_options_io *io);

uint32_t classic_original_raster_option_u32(const unsigned char *descriptor,
                                             unsigned offset);
#endif
