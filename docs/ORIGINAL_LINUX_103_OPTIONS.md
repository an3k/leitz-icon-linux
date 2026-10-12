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
| DebugMode=Debug | Debug logging/file support | Enables optional per-page/debug USB diagnostics; no persistent files |
| PrintThroughPRN=ON | Spools via debug PRN file then forwards | Validated temporary `.prn` USB staging; no job replay or persistent debug file |
| ContinuousMedia=ON | Enables roll-material processing; ContinuousPaperLength optionally specifies its length in 300dpi units | Exposed; Custom/roll lengths stay job-dependent; explicitly supplied ContinuousPaperLength sets a validated device length and extends feed as necessary |
| ShouldCorrectMargins=YES | Alters start of raster region only with ContinuousMedia ON | For explicit ContinuousMedia=ON, masks the first 30/60 dots as white while retaining physical media feed; adaptation, not original crop semantics |
| LabelCutter=ON/OFF | Changes between-page feed/cutter behavior | Existing shared LabelCutter option |
| CutMedia=True/False | Emits setpagedevice /CutMedia; original Linux raster filter does not read the option | Exposed; explicit True forces a per-page cut in the shared job controller; existing LabelCutter retained |
| ShouldSend_ESC_A=YES/NO | Exposed in PPD; not read by original Linux raster2printer.c | Exposed; YES retains USB readiness preflight, explicit NO skips only ESC A (not SKU validation) |
| CustomPageSize and LandscapeOrientation | Original accepts custom orientation 0–3 and declares Plus90 | Original PPD values restored; visual orientation still needs hardware qualification |

The original options are **not** proof of equivalent hardware wire framing:
Current keeps its own packet encoding; Native keeps its separate original-derived
encoder. Original defaults (300x300 dpi, StandardAddressLabel, LabelCutter OFF and
OptimizationLevel 2) are restored in the PPD; all original Current modes remain
selectable. Added options and modes are **not hardware-qualified**.

Observed unresolved hardware issues: 88×55mm A/B/G were rotated and too long;
G also clipped on the right. This patch does not claim to fix those CUPS
orientation or Native layout issues.

## Original macOS PrintQuality and Windows Halftone

The 15 macOS PrintQuality names, four standard Unidrv Halftone names, and both
existing LeitzDither names remain selectable in the PPD. The macOS PPD emits
24-bit RGB and delegates algorithms to a proprietary macOS filter. Only that
PPD, not the macOS algorithm implementation, was provided for this change.
This Linux implementation therefore uses supported 8-bit K/gray raster data
and source-independent dithering approximations for nontrivial quality names.
It **must not** be described as pixel-identical macOS or Windows Unidrv output.

Explicit PrintQuality takes precedence over Halftone and LeitzDither; an
explicit Halftone takes precedence over LeitzDither. Original macOS default
is PrintQuality Bayer8x8, the Unidrv default is HT_PATSIZE_SUPERCELL_M.
Legacy LeitzDither Bayer4/Threshold choices are retained and usable.

The Windows GPD imports StdNames.gpd for standard UI string IDs and names
LeitzUSRes.dll as its vendor-specific resource DLL. For the exact localized
display strings and resource DLL install path, use the Windows INF and
resource files; neither was included with the uploaded GPD.

## Original vendor status and geometry

The PPD includes all 12 distinct cupsIPPReason status IDs from Linux 1.0.3
and macOS 1.2.190. Each corresponds to a manufacturer display string; it is
**not** a declaration that the backend can detect all 12 states from ESC A.
The duplicate original com.leitz.general-error has different descriptions
in Linux and macOS; the original Linux description is retained.

The original LandscapeOrientation=Plus90, HWMargins=3.6 pt and custom
Orientation 0..3 are restored without changing source-backed per-format
PageSize, PaperDimension, ImageableArea or the per-job cut and feed logic.
