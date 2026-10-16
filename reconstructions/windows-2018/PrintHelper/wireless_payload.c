#include "wireless_payload.h"

static void zeroize(uint8_t *p,size_t size)
{
    volatile uint8_t *dst=p;
    while(size--) *dst++=0;
}

int leitz_win2018_prepare_wireless_send(
    const uint16_t *wide_text,size_t wide_capacity,
    uint8_t *scratch,size_t scratch_capacity,
    leitz_win2018_ansi_convert convert,
    leitz_win2018_wireless_write send,void *context)
{
    if (!wide_text || !wide_capacity || !scratch || !scratch_capacity ||
        !convert || !send) return -1;
    /* OEM first obtains the UTF-16 NUL-terminated length, then
     * WideCharToMultiByte(CP_ACP,0,...,units+1) to include the NUL.
     * Never read beyond the caller's known UTF-16 input capacity. */
    size_t units=0;
    while(units<wide_capacity && wide_text[units]) ++units;
    if(units==wide_capacity) return -1;

    size_t produced=0;
    int converted=convert(context,wide_text,units+1,
                          scratch,scratch_capacity,&produced);
    int result=0;
    if(converted && produced && produced<=scratch_capacity &&
       scratch[produced-1]==0) {
        /* Original calls strlen on the converted byte string and
         * passes only that many bytes to its WriteFile helper.
         * Embedded converted NUL also truncates the transfer. */
        size_t send_length=0;
        while(send_length<produced && scratch[send_length]) ++send_length;
        result=send(context,scratch,send_length) ? 1 : 0;
    } else if(converted && produced>scratch_capacity) {
        result=-1;
    }
    /* Safer than the OEM: clear temporary network configuration bytes
     * even if conversion or delivery fails. */
    zeroize(scratch,scratch_capacity);
    return result;
}
