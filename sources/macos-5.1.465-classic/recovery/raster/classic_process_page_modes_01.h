#ifndef CLASSIC_PROCESS_PAGE_MODES_01_H
#define CLASSIC_PROCESS_PAGE_MODES_01_H
#include <stddef.h>
#include <stdint.h>

/* Isolated OEM _ProcessPage modes 0/1:
 * 0x100007633..0x100007739, 0x100007939..0x100007a74,
 * 0x100007e6e..0x100007f32 and 0x10000889b..0x100008945.
 * This is NOT the original complete _ProcessPage ABI or implementation.
 */
typedef struct {
  unsigned optimization;
  unsigned packed_stride;
  uint16_t pending_empty_rows;
} classic_mode01_state;

/* Caller-provided CUPS packed row stride; no arbitrary fixed print length. */
void classic_mode01_begin(classic_mode01_state *state,unsigned mode,
                          unsigned packed_stride,uint8_t output[4]);
/* Output buffer must hold at least 4+packed_stride bytes. Return number of
 * emitted bytes, 0 for a deferred blank row in mode 1.
 * The observed original does not flush trailing deferred rows. */
size_t classic_mode01_row(classic_mode01_state *state,
                          const uint8_t *row,uint8_t *output);
#endif
