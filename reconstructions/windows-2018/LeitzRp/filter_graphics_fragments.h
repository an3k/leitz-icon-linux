#ifndef LEITZ_WIN2018_FILTER_GRAPHICS_FRAGMENTS_H
#define LEITZ_WIN2018_FILTER_GRAPHICS_FRAGMENTS_H
#include <stddef.h>
#include <stdint.h>

/* Windows x64 LeitzRp.dll 2.17.4.112. Portable recovered subpath only.
 * These interfaces are NEW test interfaces, NOT the manufacturer's COM ABI.
 * No device I/O is performed by this module.
 */
#define LEITZ_WIN2018_E_FAIL ((int32_t)0x80004005u)
typedef int32_t (*leitz_win2018_write_fn)(void *, const uint8_t *, size_t);

typedef struct {
    /* Previous trimmed row width: original internal WORD at +0x08. */
    uint16_t previous_trimmed_width;
} leitz_win2018_row_state;

/* OEM code RVAs 0x26df..0x274b: ESC D when width changes; ESC B always.
 * Input bounds checks here are deliberately safer than unspecified OEM
 * underflow handling, not a claim of exact exceptional-case parity.
 */
int32_t leitz_win2018_emit_row_prefix(leitz_win2018_row_state *state,
                                     size_t raster_length,
                                     size_t leading_blank_bytes,
                                     leitz_win2018_write_fn write,
                                     void *context);

/* OEM 0x2907..0x29a7: select already-compressed vs raw row.
 * Compressed data generation is NOT reconstructed by this function.
 */
int32_t leitz_win2018_emit_row_payload(const uint8_t *raw, size_t raw_length,
                                      const uint8_t *compressed,
                                      size_t compressed_length,
                                      leitz_win2018_write_fn write,
                                      void *context);
#endif
