#ifndef LEITZ_WIN2018_SENDRECV_SEQUENCE_H
#define LEITZ_WIN2018_SENDRECV_SEQUENCE_H
#include <stddef.h>
#include <stdint.h>

/* Portable callback reconstruction from PrintHelper.dll 2018 i386
 * SendRecvData RVA 0x1720..0x191d. Not the Windows COM/native export ABI.
 * Roles, slots, CLSID/IID and literal names derive from the documented
 * original instructions; private COM method semantics are NOT known.
 */
typedef enum {
    LEITZ_WIN2018_COM_FIRST = 1,
    LEITZ_WIN2018_COM_SECOND = 2
} leitz_win2018_com_role;

typedef struct {
    void *context;
    /* Success=0. All other results take a safe failure path, even in
     * the original code paths which dereference unchecked COM objects. */
    int32_t (*initialize)(void *context);
    int32_t (*create)(void *context, leitz_win2018_com_role role,
                      const char *clsid, const char *iid);
    /* Native vtable BYTE offset is exposed, without naming private
     * methods. argument is opaque target, UTF-16 name or payload;
     * byte_count is nonzero only for the explicit UTF-16/payload
     * buffers. constant_arg is 2 or 7 at the two proven call sites.
     * status_out is used at SECOND+0x14; reply_out/length_out at
     * SECOND+0x18. Other output pointers are NULL. */
    int32_t (*invoke)(void *context, leitz_win2018_com_role role,
                      uint32_t vtable_offset, const void *argument,
                      size_t byte_count, uint32_t constant_arg,
                      int32_t *status_out, const uint8_t **reply_out,
                      size_t *length_out);
    /* The returned COM source is released BEFORE the objects, even if
     * it could not be copied into the caller's bounded destination. */
    void (*free_reply)(void *context, const uint8_t *reply);
    void (*release)(void *context, leitz_win2018_com_role role);
    void (*uninitialize)(void *context);
} leitz_win2018_sendrecv_com_ops;

typedef struct {
    const void *original_first_argument; /* native +0x08; opaque */
    const uint8_t *request_bytes;         /* native +0x0c; byte buffer */
    size_t request_length;                /* native +0x10 */
    uint8_t *response;                    /* bounded replacement for +0x14 */
    size_t response_capacity;
    size_t *response_length;              /* replacement for +0x18 */
} leitz_win2018_sendrecv_request;

/* 1: successful, nonempty reply copied; 0: no successful reply;
 * -1: invalid inputs or reply longer than caller capacity.
 * Response and published length are unchanged on 0/-1.
 * All acquired callback resources are released in reverse order.
 */
int leitz_win2018_sendrecv_sequence(
    const leitz_win2018_sendrecv_com_ops *ops,
    const leitz_win2018_sendrecv_request *request);
#endif
