#include "status_dispatch.h"

static int utf16_equals_ascii(const uint16_t *actual, const char *expected)
{
    while (*expected) {
        if (*actual != (uint16_t)(uint8_t)*expected)
            return 0;
        ++actual;
        ++expected;
    }
    return *actual == 0;
}

int leitz_win2018_lm_dispatch(const uint16_t *request,
                              uint8_t *response, size_t response_capacity,
                              size_t *response_size,
                              leitz_win2018_lm_exchange_fn exchange,
                              void *context)
{
    if (!request || !response_size || !exchange ||
        (response_capacity && !response))
        return -1;

    /* Native x64 RVA 0x100d..0x1094 compares via lstrcmpW
     * against exactly these three UTF-16 resource strings,
     * RVAs 0x34d0, 0x34f8 and 0x3520.
     * Native unknown branch returns FALSE without touching output. */
    uint8_t second;
    if (utf16_equals_ascii(request,"leitz_oem_status")) {
        second = 'A';
    } else if (utf16_equals_ascii(request,"leitz_oem_cartridge")) {
        second = 'U';
    } else if (utf16_equals_ascii(request,"leitz_oem_version")) {
        second = 'V';
    } else {
        return 0;
    }
    const uint8_t command[] = {0x1b, second};
    size_t received = 0;
    int32_t result = exchange(context, command, sizeof command,
                              response, response_capacity, &received);

    /* Original 0x1043 or 0x102b (common branch), writes the worker's
     * numeric count even when it is zero, then returns BOOL TRUE.
     * The OEM may ignore WritePort status; a safe failure response
     * cannot claim response bytes from a failed exchange. */
    if (result < 0 || received > response_capacity ||
        received > LEITZ_WIN2018_LM_MAX_REPLY)
        received = 0;
    *response_size = received;
    return 1;
}

int leitz_win2018_lm_collect(leitz_win2018_lm_collector *state,
                              const uint8_t *packet, size_t packet_size,
                              size_t expected_length)
{
    if (!state || (packet_size && !packet) ||
        state->length > LEITZ_WIN2018_LM_MAX_REPLY ||
        expected_length > LEITZ_WIN2018_LM_MAX_REPLY)
        return -1;
    if (state->complete)
        return 1;
    if (packet_size > LEITZ_WIN2018_LM_MAX_REPLY - state->length)
        return -1;

    /* OEM 0x19a7 skips zero-sized chunks. The original stack
     * read initially requests 0x3ff, then copies to a 0x400
     * internal reply array; unlike the OEM's full-capacity
     * repeated request, this API prevents buffer overflow. */
    for (size_t i=0;i<packet_size;++i)
        state->bytes[state->length+i] = packet[i];
    state->length += packet_size;
    if (packet_size && (state->length == expected_length ||
                        packet[packet_size-1] == 0)) {
        state->complete = 1;
        return 1;
    }
    return 0;
}
