#ifndef LEITZ_WIN2018_LM_STATUS_H
#define LEITZ_WIN2018_LM_STATUS_H
#include <stddef.h>
#include <stdint.h>

#define LEITZ_WIN2018_LM_MAX_REPLY 1024u
#define LEITZ_WIN2018_LM_READ_SIZE 1023u
#define LEITZ_WIN2018_LM_WAIT_MS 5000u

/* Portable source-level interfaces, NOT the manufacturer's exported
 * language-monitor ABI. Windows WCHAR is 16-bit, unlike Linux wchar_t. */
typedef int32_t (*leitz_win2018_lm_exchange_fn)(
    void *context, const uint8_t *command, size_t command_size,
    uint8_t *response, size_t response_capacity, size_t *received);

/* LeitzLm.dll x64 RVA 0x1000: exact case-sensitive UTF-16 request IDs.
 * Returns 1 for a recognized OEM request (even if the exchange fails),
 * 0 for unknown request without any callback or length modification,
 * -1 for invalid pointers. Recognized command length is always 2.
 * This adapter bounds buffers and avoids native unsafe thread aborts.
 */
int leitz_win2018_lm_dispatch(const uint16_t *request,
                              uint8_t *response, size_t response_capacity,
                              size_t *response_size,
                              leitz_win2018_lm_exchange_fn exchange,
                              void *context);

/* Isolated portion of native asynchronous ReadPort worker
 * (x64 RVA 0x1860..0x1aee). The actual Win32 thread, event and timeout
 * are deliberately outside this offline, deterministic collector. */
typedef struct {
    uint8_t bytes[LEITZ_WIN2018_LM_MAX_REPLY];
    size_t length;
    int complete;
} leitz_win2018_lm_collector;

/* 1: complete, 0: continue reading, -1: invalid/overflow.
 * At OEM 0x19fb / 0x1a05 the loop completes when total bytes equal
 * the expected length OR the last received byte is NUL.
 */
int leitz_win2018_lm_collect(leitz_win2018_lm_collector *state,
                              const uint8_t *packet, size_t packet_size,
                              size_t expected_length);
#endif
