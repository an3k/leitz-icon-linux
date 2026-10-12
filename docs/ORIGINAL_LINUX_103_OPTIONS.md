# Original Linux 1.0.3: option inventory and Current mapping

Source: uploaded `LeitzIconDriversForLinux1.0.3.zip`:
`PPD/LeitzIcon.ppd.gz`, `raster2printer.c`,
`raster_c_functions.c`. Original sources and Native remain untouched.

| Original option | Actual 1.0.3 behavior | Current implementation |
| --- | --- | --- |
| PageSize/PageRegion | 40 named formats in original Linux PPD | 86 catalog profiles + Custom; unchanged |
| Resolution=300dpi/600dpi | 300×300/300×600; ESC h/i | 300x300dpi/300x600dpi; existing |
| OptimizationLevel=0 | Full-width raw, **including all blank rows** | LeitzCurrentRaster=NoSkipRaw (new, experimental) |
| OptimizationLevel=1 | Skip blank rows, otherwise full-width raw | LeitzCurrentRaster=FullWidthRaw (existing, experimental) |
| OptimizationLevel=2 | Skip blank rows and trim blank byte margins of nonblank rows | LeitzCurrentRaster=TrimmedRaw (existing, experimental) |
| OptimizationLevel=3 | Same code path as level 1 in supplied source; an extra level-3 condition is unreachable inside level-2-only branch | No duplicate selector |
| DebugMode=Release | Unmodified raster | Default |
| DebugMode=Mesh | Mark left/middle/right and first/middle/last line | DebugMode=Mesh adapted to normalized raster (new, diagnostic) |
| DebugMode=Debug | Debug logging/file support | Not added; no unnecessary files in production |
| PrintThroughPRN=ON | Spools via debug PRN file then forwards | Not added; Current already validates and spools before USB |
| ContinuousMedia=ON | Switches label-length processing; ContinuousPaperLength option supplies length | Automatic job length and CUPS Custom; original fixed-length behavior not copied |
| ShouldCorrectMargins=YES | Alters start of raster region only with ContinuousMedia ON | Not treated as equivalent to Current margin geometry |
| LabelCutter=ON/OFF | Changes between-page feed/cutter behavior | Existing shared LabelCutter option |
| CutMedia=True/False | Emits setpagedevice /CutMedia in PPD; original raster filter never reads it | Not duplicated; LabelCutter is operative control |
| ShouldSend_ESC_A=YES/NO | Exposed in PPD; not read by original raster2printer.c | Not duplicated; USB readiness check retained |
| CustomPageSize and LandscapeOrientation | Original accepts custom orientation 0–3 and declares Plus90 | Current orientation configuration unchanged pending real raster/rotation evidence |

The original options are **not** proof of equivalent hardware wire framing:
Current keeps its own packet encoding; Native keeps its separate original-derived
encoder. Current's Optimized default and all existing printed 40/80 mm workflows
remain unchanged. Added modes are **not hardware-qualified**.

Observed unresolved hardware issues: 88×55mm A/B/G were rotated and too long;
G also clipped on the right. This patch does not claim to fix those CUPS
orientation or Native layout issues.
