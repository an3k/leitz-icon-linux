#ifndef LEITZ_WIN2018_DEVMODE_PATCH_H
#define LEITZ_WIN2018_DEVMODE_PATCH_H
#include <stddef.h>
#include <stdint.h>

/* Safe portable reconstruction of PrintHelper.dll 2018 (i386)
 * SetPaper RVA 0x1b30 and SetCustomPaper RVA 0x1930 DEVMODE writes.
 * NOT Windows DEVMODE API, SetPrinter, Windows COM or device protocol.
 */
#define LEITZ_WIN2018_DM_PAPERSIZE (0x00000002u)
#define LEITZ_WIN2018_DM_ORIGINAL_FLAGS (0x00002402u)

typedef struct {
    uint16_t paper_size;
    uint16_t print_quality;
    uint16_t y_resolution;
    uint16_t private_minus_16;
    uint16_t private_minus_14;
    uint16_t private_minus_10;
} leitz_win2018_devmode_values;

/* 1 patched, 0 original SetPaper precondition failed (no edits),
 * -1 bad/incomplete DEVMODE. Unknown Windows spooler side effects
 * are deliberately not reproduced. All writes are little-endian.
 */
int leitz_win2018_patch_set_paper(uint8_t *devmode, size_t bytes,
                                  const leitz_win2018_devmode_values *values);
int leitz_win2018_patch_custom_paper(uint8_t *devmode, size_t bytes,
                                     const leitz_win2018_devmode_values *values);
#endif
