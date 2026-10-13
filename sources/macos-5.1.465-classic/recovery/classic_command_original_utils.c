#include "classic_command_original_utils.h"

/* _is_bit_set at 0x100001c60: x86 shl by CL (masked to five low
 * bits for 32-bit operand), and/compare against zero. */
int classic_command_original_is_bit_set(unsigned value, unsigned bit) {
    unsigned mask = 1u << (bit & 31u);
    return (value & mask) != 0u;
}

/* _FlushDataStreams 0x100001c20..0x100001c5c:
 * unconditional stdout fflush; optional __fd_dump fflush; return ignored. */
void classic_command_original_flush(FILE *out, FILE *dump) {
    fflush(out);
    if (dump) fflush(dump);
}

/* _CloseDebugFile 0x100002440..0x100002475:
 * fclose and set __fd_dump to null, without checking fclose result. */
void classic_command_original_close_debug(FILE **dump) {
    if (*dump) {
        fclose(*dump);
        *dump = NULL;
    }
}

/* _CloseLogFile 0x100002480..0x1000024b5:
 * fclose and set __fd_log to null, without checking fclose result. */
void classic_command_original_close_log(FILE **log) {
    if (*log) {
        fclose(*log);
        *log = NULL;
    }
}
