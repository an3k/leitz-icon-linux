/* Original 5.1.465 raster2printer Intel Mach-O; SHA-256:
 * 72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13.
 * Source: _raster2printer exit 0x1000062eb..0x10000685a.
 * Directly follows the normal-page processing loop. Native calls
 * pass the whole 0x10a8-byte descriptor by value; per-call copies
 * reproduce that observable transfer without attempting to fabricate
 * the original C++/SysV x86_64 calling convention. */
#include "original_raster_job_epilogue.h"
#include <stdio.h>
#include <string.h>

/* Source-level bridge for repeated original native global reads:
 * 0x1000064e1: test __ZL4sock against -1 before close
 * 0x1000064ee: reload __ZL4sock as close(fd)
 * 0x1000065e1: reload __ZL4sock for terminated reset _WriteData
 * 0x10000669a: reload __ZL4sock for trailing ESC _WriteData.
 * Values may change between calls; avoid an entry-only snapshot.
 * Verified direct original disassembly CI 38784545750.
 */
static int original_socket_now(
    const classic_original_raster_finish_ops *ops,
    const classic_original_raster_finish_state *state) {
    return ops->current_socket_fd ?
        ops->current_socket_fd(ops->context) : state->socket_fd;
}
/* OEM original _raster2printer 0x100006504:
 * movl _terminate_flag(%rip),%eax, AFTER close/PRN cleanup,
 * 0x10000650a compares to zero, 0x10000650d skips ESC @ ESC Z if zero.
 * Only read the live flag at this exact original decision site. */
static int original_terminated_now(
    const classic_original_raster_finish_ops *ops,
    const classic_original_raster_finish_state *state) {
    return ops->current_terminated ?
        ops->current_terminated(ops->context) : state->terminated;
}

int classic_original_raster_finish_job(
    const classic_original_raster_finish_ops *ops,
    const classic_original_raster_finish_state *state) {
    uint8_t copy[CLASSIC_OEM_DESCRIPTOR_BYTES];
    memcpy(copy, state->descriptor, sizeof copy);
    ops->end_document(ops->context, copy, sizeof copy);

    /* OEM compares to zero, not to -1 or a boolean flag. */
    if (state->raster_fd != 0)
        (void)ops->close_fd(ops->context, state->raster_fd);
    ops->close_debug_file(ops->context);

    uint32_t print_prn = 0;
    memcpy(&print_prn, state->descriptor + 8, sizeof print_prn);
    if (print_prn != 0) {
        (void)ops->send_debug_prn(ops->context);
        fprintf(stderr,
            "\nraster2printer.c:440:raster2printer(): print through PRN file ");
    } else {
        fprintf(stderr,
            "\nraster2printer.c:444:raster2printer(): do not print through PRN file ");
    }

    /* Original 0x1000064b2..0x100006504: scheme substring, then
     * the nonzero 8-byte descriptor slot +0x1090 and socket != -1.
     * Do not infer the semantic meaning of the native slot. */
    const char *uri = (const char *)state->descriptor + 0x50;
    uint64_t slot_1090=0;
    memcpy(&slot_1090, state->descriptor + 0x1090, sizeof slot_1090);
    if (strstr(uri, "ipp") != NULL && slot_1090 != 0 &&
        original_socket_now(ops,state) != -1)
        (void)ops->close_fd(ops->context, original_socket_now(ops,state));

    if (original_terminated_now(ops,state)) {
        /* Manufacturer builds four command bytes ESC @ ESC Z, appends two
         * NUL bytes and writes all SIX bytes (0x100006513..0x100006630).
         * This sequence must not be shortened to four bytes. */
        static const uint8_t reset_and_end[6] = {0x1b, '@', 0x1b, 'Z', 0, 0};
        *state->job_size += 6;
        memcpy(copy, state->descriptor, sizeof copy);
        (void)ops->write_data(ops->context, reset_and_end,
                              sizeof reset_and_end, copy, sizeof copy,
                              original_socket_now(ops,state));
    }

    /* Original signed idivl by 64; OEM sends one ESC even when the
     * global job_size is zero and does not increment the counter. */
    if ((int32_t)(*state->job_size) % 64 == 0) {
        static const uint8_t esc = 0x1b;
        memcpy(copy, state->descriptor, sizeof copy);
        (void)ops->write_data(ops->context, &esc, 1,
                              copy, sizeof copy, original_socket_now(ops,state));
    }
    fprintf(stderr,"\nSTATE: none\n");
    fprintf(stderr,
        "\nraster2printer.c:471:raster2printer():  job size : %i ",
        (int32_t)*state->job_size);
    fprintf(stderr,
        "\nraster2printer.c:472:raster2printer(): raster filter finished\n");
    return state->return_code;
}
