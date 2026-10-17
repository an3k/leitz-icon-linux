#ifndef LEITZ_WIN2018_OEM_ROW_DISPATCH_H
#define LEITZ_WIN2018_OEM_ROW_DISPATCH_H
#include "raster_stream.h"

/* LeitzRp OEM raster method #15: x86 RVA 0x1FF0, x64 RVA 0x2040.
 * The original first increments the 16-bit row index (x86 medium +0x0A,
 * x64 medium +0x0E), then branches on a DWORD IsDieCut flag (x86 +0x10,
 * x64 +0x14). These offsets refer to ORIGINAL native objects, not
 * the portable leitz_win2018_row_control struct.
 * IsDieCut != 0 selects the already reconstructed counted-row stream;
 * IsDieCut == 0 selects a different OEM routine (RVA 0x2380) whose full
 * implementation has NOT been reconstructed. Supply that path as a
 * separate callback; no guessed output is ever emitted instead.
 */
typedef int32_t (*leitz_win2018_oem_alt_row)(
    void *context, leitz_win2018_raster_stream *stream,
    const uint8_t *pixels, uint32_t length);

/* Deliberately validate required portable inputs before native-style
 * row-count increment; this avoids an unsafe partial update for a
 * missing fallback implementation. Once dispatch begins, row_index
 * increments even when the selected implementation returns failure.
 * The original 16-bit counter wraps; successful label end may reset it.
 * The returned value is the selected branch's result, not an invented
 * combined or normalized COM HRESULT.
 */
#define LEITZ_WIN2018_OEM_ALT_PATH_MISSING ((int32_t)0x80004001u)
int32_t leitz_win2018_oem_dispatch_row(
    leitz_win2018_raster_stream *stream,
    const uint8_t *pixels, uint32_t length,
    uint32_t is_die_cut,
    leitz_win2018_oem_alt_row alternate, void *alternate_context);
#endif
