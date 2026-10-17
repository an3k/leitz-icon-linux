#include "sendrecv_sequence.h"
#include "sendrecv_result.h"

/* Original manufacturer class/interface pairs (not fabricated COM APIs). */
static const char class_first[] = "2a614240-a4c5-4c33-bd87-1bc709331639";
static const char iid_first[]   = "d580dc0e-de39-4649-baa8-bf0b85a03a97";
static const char class_second[]= "b9162a23-45f9-47cc-80f5-fe0fe9b9e1a2";
static const char iid_second[]  = "8f348bd7-4b47-4755-8a9d-0f422df3dc89";
/* The actual OEM operands are UTF-16. Do not truncate ReadWriteUserData. */
static const uint16_t operation_name[] = {
    'R','e','a','d','W','r','i','t','e','U','s','e','r','D','a','t','a',0
};
static const uint16_t get_name[] = {'G','e','t',0};

int leitz_win2018_sendrecv_sequence(
    const leitz_win2018_sendrecv_com_ops *ops,
    const leitz_win2018_sendrecv_request *request)
{
    if (!ops || !request || !ops->initialize || !ops->create ||
        !ops->invoke || !ops->free_reply || !ops->release ||
        !ops->uninitialize || !request->original_first_argument ||
        !request->response_length || !request->response ||
        (request->request_length && !request->request_bytes))
        return -1;

    int result = 0;
    int started = 0, first_created = 0, second_created = 0;
    const uint8_t *reply = NULL;
    size_t reply_length = 0;
    int32_t reported_status = 0;

    if (ops->initialize(ops->context) != 0)
        return 0;
    started = 1;
    if (ops->create(ops->context, LEITZ_WIN2018_COM_FIRST,
                    class_first, iid_first) != 0)
        goto done;
    first_created = 1;
    if (ops->invoke(ops->context, LEITZ_WIN2018_COM_FIRST, 0x0cu,
                    request->original_first_argument, 0, 2u,
                    NULL, NULL, NULL) != 0)
        goto done;
    if (ops->create(ops->context, LEITZ_WIN2018_COM_SECOND,
                    class_second, iid_second) != 0)
        goto done;
    second_created = 1;
    if (ops->invoke(ops->context, LEITZ_WIN2018_COM_SECOND, 0x0cu,
                    operation_name, sizeof operation_name, 0,
                    NULL, NULL, NULL) != 0)
        goto done;
    if (ops->invoke(ops->context, LEITZ_WIN2018_COM_SECOND, 0x10u,
                    request->request_bytes, request->request_length, 7u,
                    NULL, NULL, NULL) != 0)
        goto done;
    if (ops->invoke(ops->context, LEITZ_WIN2018_COM_FIRST, 0x14u,
                    get_name, sizeof get_name, 0,
                    NULL, NULL, NULL) != 0)
        goto done;
    if (ops->invoke(ops->context, LEITZ_WIN2018_COM_SECOND, 0x14u,
                    NULL, 0, 0, &reported_status, NULL, NULL) != 0 ||
        reported_status != 0)
        goto done;
    const int32_t data_result = ops->invoke(
        ops->context, LEITZ_WIN2018_COM_SECOND, 0x18u, NULL, 0, 0,
        NULL, &reply, &reply_length);
    if (data_result == 0)
        result = leitz_win2018_sendrecv_copy_result(
            0, reported_status, data_result, reply, reply_length,
            request->response, request->response_capacity,
            request->response_length);

done:
    if (reply)
        ops->free_reply(ops->context, reply);
    if (second_created)
        ops->release(ops->context, LEITZ_WIN2018_COM_SECOND);
    if (first_created)
        ops->release(ops->context, LEITZ_WIN2018_COM_FIRST);
    if (started)
        ops->uninitialize(ops->context);
    return result;
}
