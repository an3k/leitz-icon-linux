#ifndef LEITZ_WIN2018_PAPER_GEOMETRY_H
#define LEITZ_WIN2018_PAPER_GEOMETRY_H
#include <stddef.h>
#include <stdint.h>
/* New portable test API, not the native OEM COM/Windows ABI.
 * Native paper IDs occupy 4-byte cells, with the low WORD selected
 * at base + index*4 (RVA 0x13d3); this API accepts compact uint16_t IDs.
 * Win32 PrintHelper.dll GetPaperDefs RVA 0x1210..0x1500:
 * SetPrinterW chooses each paper, then GetDeviceCaps reads six values.
 * Neither device nor printer settings are accessed by this module. */
typedef int (*leitz_win2018_select_paper_fn)(void *, uint16_t);
typedef int (*leitz_win2018_get_cap_fn)(void *, int, int32_t *);
enum leitz_win2018_device_cap {
  LEITZ_WIN2018_HORZRES=8, LEITZ_WIN2018_VERTRES=10,
  LEITZ_WIN2018_PHYSICALWIDTH=110, LEITZ_WIN2018_PHYSICALHEIGHT=111,
  LEITZ_WIN2018_PHYSICALOFFSETX=112, LEITZ_WIN2018_PHYSICALOFFSETY=113
};
typedef struct {
  uint16_t paper_id; /* Not a cartridge/SKU identifier. */
  int32_t physical_offset_x, physical_offset_y;
  int32_t physical_width, physical_height;
  int32_t printable_raster_width, printable_raster_height;
} leitz_win2018_paper_geometry;
/* Callbacks are mockable, not real SetPrinterW; paper IDs are supplied,
 * never inferred. GDI values are raster/device units rather than mm.
 * Returns 0 on success, -1 for bad inputs or callback failure. Records
 * are committed only after all six values for a paper are obtained. */
int leitz_win2018_collect_paper_geometry(
  const uint16_t *, size_t, leitz_win2018_paper_geometry *, size_t,
  leitz_win2018_select_paper_fn, leitz_win2018_get_cap_fn, void *);
#endif
