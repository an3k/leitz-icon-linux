#ifndef CLASSIC_ORIGINAL_STATUS_DECODER_H
#define CLASSIC_ORIGINAL_STATUS_DECODER_H
#include <stddef.h>

/* Isolated classification and requested side effects from the manufacturer's
 * _CheckPrinterStatus 0x100009511..0x1000099f2. The original native IO,
 * output/retry loop and return ABI are NOT implemented by this helper. */
typedef struct {
    const char *state;
    const char *info;
    unsigned send_reset_and_end;
    unsigned sleep_seconds;
} classic_original_status_decision;

/* A positive reply must be original NUL-terminated status text.
 * received <= 0 denotes the observed error/empty branches.
 * This helper does not add new state mappings or fallback conditions.
 */
classic_original_status_decision
classic_original_decide_status(const char *reply,long received);
#endif
