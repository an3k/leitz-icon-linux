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

## Source-backed macOS cartridge and page compatibility

The new `media/vendor_macos_catalog.json` was extracted directly from the
uploaded macOS `CartridgesCatalog.plist` (23 SKUs) and `PaperCatalog.plist`
(37 catalog format records). This explicitly distinguishes cartridge physical
width, adhesive/stock type, format compatibility, and orientation metadata.
The source reports SKU `170030001` (9 digits), therefore ESC U also accepts
a nine-digit SKU where present.

The CUPS USB backend uses the vendor's `CompatibleSKUs` for an explicitly
selected PPD `PageSize` when a macOS catalog entry exists, instead of guessing
an SKU from the page name. For formats absent from that macOS source the
existing strictly bounded print-width checks remain in force. The normal
PPD default for `IsCurrentInsertedCartridgeDieCut` becomes `Auto`, while
the original YES and NO remain available and enforce the actual stock
classification when explicitly selected. This prevents the original hardcoded
YES default from incorrectly refusing roll-material jobs.

The vendor `validate_reception.py` also confirms read-only USB queries
ESC A (status), ESC V (version), and ESC u 00 (Wi-Fi state). The CUPS command
filter and backend now support `ReportStatus`, `GetPrinterStatus`,
`GetFirmwareVersion` but deliberately do **not** expose Wi-Fi or boot-mode controls:
the script's default PID 0x1001 and the macOS `LeitzIconV3.drv`
describe the separate Icon 900 Pro Turbo/V3 model.
These read-only commands are source-backed but not yet hardware-verified.

The full uploaded Windows/macOS archive inventories and both LeitzUSRes DLLs
have subsequently been inspected. The application archives contain many
template images and localization assets but do not make the CUPS filter a
template renderer. Pixel-identical proprietary Windows/macOS algorithms
remain unverified where their original arithmetic is unavailable.

## Review of full uploaded 2026-10-12 vendor archives

The Windows OEMPRINT.INF identifies the v3 Unidrv driver (2.17.4.112),
LeitzLm.dll (language monitor), LeitzRp.dll (rendering plug-in),
LeitzUI.dll, LeitzDevMode.dll and LeitzUSRes.dll (resource strings).
The resource DLL provides actual English label names; standard Halftone
names refer to Windows Unidrv rather than Leitz-specific Dither kernels.
The macOS leitzprint.pkg contains command2leitz, command2leitzV3,
raster2printer and raster2printerV3. The V3 binary has additional
cartridge, Wi-Fi and status branches for the **separate Icon 900 Pro
Turbo**. They are not assumptions about the original Icon USB (0x1000).

In raster2printer the macOS 1.2.190 x86_64 Mach-O __DATA,__data
contains exact manufacturer matrices for SuperCell (8x8),
Bayer4x4, Bayer8x8, Bayer14x14, Clustered8x8,
Dispersed8x8, Nasik4x4, and Dithering3x3. Those eight original
TABLES are preserved in src/leitz_vendor_dither.h and selected by
PrintQuality in Current. Threshold scaling is a source-based
adaptation; complete macOS DitherPage arithmetic has not yet been
reconstructed. Other PrintQuality methods still use the documented
Current approximations and are NOT original-equivalent.

The vendor Python validate_reception.py documents the read-only
ESC V firmware response, ESC u 00 Wi-Fi status, and ESC A status flags.
Changing Wi-Fi state and entering boot mode are deliberately NOT
wired into printing or default maintenance.

The two label-template archives were inventoried. Their LeitzLbl
files are application-level label layouts (not printer raster
encoders); the CUPS raster filter accepts output from those apps.
No template-to-PPD assumptions are made.

## Read-only CUPS United Status (original macOS)

In both uploaded macOS `command2leitz` and `command2leitzV3` Mach-O
executables, the original function `GetUnitedStatus` passes opcode `U`
to `GetStatus`. `GetPrinterStatus` uses `A`, and
`GetFirmvareVersion` (vendor spelling) uses `V`. The backend now
recognizes these three original read-only operations and validates the
known `SKU:...;INIT:...;USED:...;` reply format. This is not a
mechanical-completion or hardware qualification claim. Network/Wi-Fi
write operations and boot mode are not exposed.

## Completed macOS compatibility inventory and media-width handling

The manufacturer macOS PaperCatalog.plist contains 37 paper format records,
not 22. The original mapping in media/vendor_macos_catalog.json was
incomplete. All 37 entries, including StandardAddressLabel,
ShippingLabel70180001, ShippingLabel70130001, PackageShippingLabel and the
archival label variants, are now represented with their exact source-listed
CompatibleSKUs, orientation flags and available IPP names. The USB backend
uses those manufacturer whitelist entries when the CUPS job supplies a named
PageSize; it does not infer SKUs from a name suffix. In the absence of a
matching catalog entry, width checks remain independent.

The following physical raster widths are supported using the intersection of
the macOS compatible SKUs and corresponding Windows GPD PrintableArea:
- SKU70120001 -> InternationalAddressLabel, 960 pixels;
- SKU70170001 -> StandardAddressLabel, 960 pixels;
- SKU70280001 -> Wristband, 306 pixels.

A printed page must still fit the physical cassette. Compatibility on paper
is a source-based validation rule, not a hardware-confirmed print outcome.
Original OptimizationLevel 0..3 is prioritized over the old default
LeitzCurrentRaster=Optimized when both defaults are present in CUPS options,
while explicit nondefault Current raw modes remain available.
