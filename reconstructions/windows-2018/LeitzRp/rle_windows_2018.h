#ifndef LEITZ_WIN2018_RLE_H
#define LEITZ_WIN2018_RLE_H
#include <stddef.h>
#include <stdint.h>

/* Portable C derived from original 2018 amd64 LeitzRp.dll RVA
 * 0x274c..0x2905 and i386 LeitzRp.dll RVA 0x2800..0x2910.
 * New API, NOT the Windows manufacturer's original COM/C++ ABI.
 */
enum leitz_win2018_rle_status {
    LEITZ_WIN2018_RLE_COMPRESSED = 0,
    LEITZ_WIN2018_RLE_USE_RAW = 1,
    LEITZ_WIN2018_RLE_INVALID = -1
};
/* raw: already-trimmed raster bytes. Capacity >= raw_length required.
 * Each code octet: high bit = run color (1=black), low seven bits =
 * run length minus one, 1..128 bits per code.
 * Return COMPRESSED only when encoded bytes are STRICTLY fewer than
 * raw bytes, as the original Windows 0x2907..0x2925 selector demands.
 * out_length = 0 for USE_RAW. No device I/O.
 */
int leitz_win2018_rle_encode(const uint8_t *raw, size_t raw_length,
                             uint8_t *out, size_t out_capacity,
                             size_t *out_length);

/* Windows x64 RVA 0x2774..0x27cc conditionally clears unused low bits
 * in the final physical-width raster byte, with IN-PLACE OEM mutation.
 * We copy first to avoid changing the caller's input; the original
 * physical_width_bits=0 exceptional path is intentionally excluded.
 */
int leitz_win2018_copy_clipped_row(const uint8_t *raw, size_t length,
                                   unsigned physical_width_bits,
                                   uint8_t *copy, size_t copy_capacity);
#endif
