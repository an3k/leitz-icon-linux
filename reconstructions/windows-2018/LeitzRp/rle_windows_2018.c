#include "rle_windows_2018.h"

static int append_run(uint8_t color, unsigned run_length,
                      uint8_t *out, size_t raw_length,
                      size_t *written)
{
    if (*written >= raw_length)
        return LEITZ_WIN2018_RLE_USE_RAW;
    out[(*written)++] = (uint8_t)((color ? 0x80u : 0u) |
                                  (uint8_t)(run_length - 1u));
    return *written >= raw_length ? LEITZ_WIN2018_RLE_USE_RAW :
                                    LEITZ_WIN2018_RLE_COMPRESSED;
}

int leitz_win2018_rle_encode(const uint8_t *raw, size_t raw_length,
                             uint8_t *out, size_t out_capacity,
                             size_t *out_length)
{
    if (!out_length || (raw_length && (!raw || !out)) ||
        out_capacity < raw_length)
        return LEITZ_WIN2018_RLE_INVALID;
    *out_length = 0;
    if (raw_length <= 1u)
        return LEITZ_WIN2018_RLE_USE_RAW;

    uint8_t run_color = 0;
    unsigned run_length = 0;
    size_t written = 0;
    for (size_t i=0; i<raw_length; ++i) {
        const uint8_t value = raw[i];
        /* Original amd64 fast path RVA 0x27fe..0x283b advances an
         * entire 00/ff byte when it fits the 128-bit run counter.
         * x86 OEM uses the same test at RVA 0x2836..0x288b. */
        if (value == 0x00u || value == 0xffu) {
            const uint8_t homogeneous_color = value == 0xffu;
            if (!run_length) {
                run_color = homogeneous_color;
                run_length = 8u;
                continue;
            }
            if (homogeneous_color == run_color && run_length <= 120u) {
                run_length += 8u;
                continue;
            }
        }

        /* Exact bit/run counting: x64 RVA 0x2860..0x28d6,
         * x86 RVA 0x2860..0x28d8, high bit first.
         * This variant avoids the OEM's unmanaged buffer writes. */
        for (unsigned mask=0x80u; mask; mask >>= 1u) {
            const uint8_t color = (value & mask) != 0;
            if (run_length && color == run_color && run_length < 128u) {
                ++run_length;
            } else {
                if (run_length && append_run(run_color,run_length,out,
                                             raw_length,&written) !=
                                             LEITZ_WIN2018_RLE_COMPRESSED)
                    return LEITZ_WIN2018_RLE_USE_RAW;
                run_color = color;
                run_length = 1u;
            }
        }
    }
    if (run_length && append_run(run_color,run_length,out,
                                 raw_length,&written) !=
                                 LEITZ_WIN2018_RLE_COMPRESSED)
        return LEITZ_WIN2018_RLE_USE_RAW;
    *out_length = written;
    return LEITZ_WIN2018_RLE_COMPRESSED;
}

int leitz_win2018_copy_clipped_row(const uint8_t *raw, size_t length,
                                   unsigned physical_width_bits,
                                   uint8_t *copy, size_t copy_capacity)
{
    if ((length && (!raw || !copy)) || copy_capacity < length ||
        physical_width_bits == 0 || physical_width_bits > UINT16_MAX)
        return LEITZ_WIN2018_RLE_INVALID;

    for (size_t i=0; i<length; ++i)
        copy[i]=raw[i];
    const unsigned remainder = physical_width_bits & 7u;
    if (length == ((size_t)physical_width_bits+7u)/8u && remainder && length)
        copy[length-1] &= (uint8_t)(0xffu << (8u-remainder));
    return 0;
}
