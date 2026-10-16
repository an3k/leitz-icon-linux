#ifndef LEITZ_WIN2018_WIFI_PASSWORD_H
#define LEITZ_WIN2018_WIFI_PASSWORD_H
#include <stddef.h>
#include <stdint.h>

/* Source: original LeitzIconAddPrinterTool.exe 2018 MethodDef 285
 * RVA 0x6c34, and its static Regex initializer MethodDef 286 0x6d18.
 * New portable offline C API, not the manufacturer's .NET signature.
 */
typedef enum {
    LEITZ_WIN2018_WIFI_PASSWORD_VALID = 0,
    LEITZ_WIN2018_WIFI_PASSWORD_REQUIRED = 1,
    LEITZ_WIN2018_WIFI_PASSWORD_INCORRECT = 2,
    LEITZ_WIN2018_WIFI_PASSWORD_INPUT_ERROR = -1
} leitz_win2018_wifi_password_result;

/* key is a NUL-terminated Windows UTF-16 code-unit array bounded by
 * capacity; security_mode==0 accepts even NULL (unsecured OEM case).
 * Missing key for other modes corresponds to IsNullOrEmpty.
 * Bounds checking is a new safety condition. Does not make a copy,
 * log the key, access the network or modify printer settings.
 */
leitz_win2018_wifi_password_result leitz_win2018_check_wifi_password(
    int32_t security_mode,const uint16_t *key,size_t capacity);
#endif
