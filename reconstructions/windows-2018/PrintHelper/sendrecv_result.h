#ifndef LEITZ_WIN2018_SENDRECV_RESULT_H
#define LEITZ_WIN2018_SENDRECV_RESULT_H
#include <stddef.h>
#include <stdint.h>

/* Portable, bounded model of the successful COM-result branch from
 * PrintHelper.dll 2018 SendRecvData x86 RVA 0x17f2..0x18ec.
 * NOT an implementation of COM, Windows HeapAlloc or the OEM ABI.
 */
/* Success=1 iff status method and data method return S_OK, the
 * status out-parameter is zero, and a nonempty reply is available.
 * No-reply / nonzero HRESULT or status=0 without changing caller output.
 * Invalid pointer/capacity=-1 without changing caller output.
 * Caller owns source and destination; this model does not take over
 * the OEM's CoTaskMemFree or HeapAlloc ownership.
 */
int leitz_win2018_sendrecv_copy_result(int32_t status_method_result,
                                       int32_t reported_status,
                                       int32_t data_method_result,
                                       const uint8_t *reply,
                                       size_t reply_length,
                                       uint8_t *destination,
                                       size_t capacity,
                                       size_t *published_length);
#endif
