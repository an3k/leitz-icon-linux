#include "sendrecv_result.h"
#include <limits.h>
#include <string.h>

int leitz_win2018_sendrecv_copy_result(int32_t status_method_result,
                                       int32_t reported_status,
                                       int32_t data_method_result,
                                       const uint8_t *reply,
                                       size_t reply_length,
                                       uint8_t *destination,
                                       size_t capacity,
                                       size_t *published_length)
{
    if (!published_length)
        return -1;
    /* Original RVA 0x17f6..0x1822 gates copying on a zero result
     * from vtable +0x14, a zero status at stack -0x18, and zero
     * result from vtable +0x18. The final return also requires the
     * COM-provided reply size at stack -0x2c to be nonzero. */
    if (status_method_result != 0 || reported_status != 0 ||
        data_method_result != 0 || reply_length == 0)
        return 0;

    /* Original RVA 0x1883..0x18b8 allocates a caller-owned heap buffer
     * sized by an unchecked COM length; this adaptation never allocates
     * and cannot overflow the caller's already-provided destination. */
    if (!reply || !destination || reply_length > capacity ||
        reply_length > UINT32_MAX)
        return -1;

    /* memmove also tolerates overlapping borrowed caller buffers;
     * unlike the OEM we never copy to an unchecked HeapAlloc result. */
    memmove(destination,reply,reply_length);
    *published_length=reply_length;
    return 1;
}
