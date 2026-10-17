#ifndef LEITZ_WIN2018_OEM_DEVMODE_H
#define LEITZ_WIN2018_OEM_DEVMODE_H
#include <stddef.h>
#include <stdint.h>
/* Original LeitzDevMode.dll x86/x64 hrOEMDevMode RVA 0x1080.
 * Portable bounded API, not Windows OEMDMPARAM/Unidrv binary ABI. */
#define LEITZ_WIN2018_OEMDEVMODE_BYTES 28u
#define LEITZ_WIN2018_OEMDEVMODE_SIGNATURE 0x4c504944u /* "DIPL" */
#define LEITZ_WIN2018_OEMDEVMODE_E_FAIL ((int32_t)0x80004005u)
typedef enum {
 LEITZ_WIN2018_OEMDM_SIZE=1, LEITZ_WIN2018_OEMDM_DEFAULT=2,
 LEITZ_WIN2018_OEMDM_CONVERT=3, LEITZ_WIN2018_OEMDM_MERGE=4
} leitz_win2018_oemdm_mode;
typedef struct {
 void (*set_last_error)(void *, uint32_t);
 void (*signature_mismatch)(void *);
 void *context;
} leitz_win2018_oemdm_callbacks;
/* Mode 1 writes size_bytes=28. Mode 2 writes five DWORDs, preserving
 * 8 trailing private bytes. Modes 3/4 compare DWORD +4, copy min
 * declared source/destination lengths on signature match and force
 * size=28 and DWORD +8=1; mismatch resets first 20 bytes. Invalid
 * inputs are safely rejected (original does not check buffer sizes).
 * E_FAIL calls optional SetLastError surrogate with code 87. */
int32_t leitz_win2018_oemdevmode(leitz_win2018_oemdm_mode mode,
 const uint8_t *source, size_t source_capacity,
 uint8_t *destination, size_t destination_capacity,
 uint32_t *size_bytes, const leitz_win2018_oemdm_callbacks *callbacks);
#endif
