#include "oem_write_data.h"
#include <stdint.h>

int32_t leitz_rp_oem_write_data(leitz_rp_oem_write_state *s,
    const uint8_t *bytes, uint32_t length)
{
    if (!s || (s->architecture != LEITZ_RP_WRITE_X86 &&
               s->architecture != LEITZ_RP_WRITE_X64) ||
        !s->write || !s->report || (length && !bytes))
        return (int32_t)LEITZ_RP_WRITE_E_FAIL;

    if (s->architecture == LEITZ_RP_WRITE_X86)
        s->accumulated_bytes = (uint32_t)(s->accumulated_bytes + length);
    else
        s->accumulated_bytes += length;

    uint32_t written = 0;
    const int32_t result = s->write(s->callback_context, s->device,
                                    bytes, length, &written);
    if (written == length && (uint32_t)result != LEITZ_RP_WRITE_E_FAIL)
        return (int32_t)LEITZ_RP_WRITE_S_OK;

    (void)s->report(s->callback_context, s->device, bytes, length,
                    LEITZ_RP_WRITE_ERROR_CODE);
    return (int32_t)LEITZ_RP_WRITE_E_FAIL;
}

int32_t leitz_rp_oem_write_callback(void *context,
    const uint8_t *bytes, size_t length)
{
    if (length > UINT32_MAX)
        return (int32_t)LEITZ_RP_WRITE_E_FAIL;
    return leitz_rp_oem_write_data((leitz_rp_oem_write_state *)context,
                                   bytes, (uint32_t)length);
}
