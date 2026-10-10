# Leitz Icon Linux — vendor porting baseline (10 Oct 2026)

This is an **evidence-based gap inventory**, not a claim of full parity. The original vendor binaries remain private and are not committed.

## Independent vendor sources examined

- Windows 2018: `LeitzIcon.gpd` (76 PaperSize entries), `LeitzRp.DLL` (OEM raster rendering), `LeitzLm.dll` (language/status monitor), `LeitzUI.DLL`.
- macOS installer 5.1.465: extracted the Apple HFS+ DMG, nested XAR distribution and gzip/ODC-CPIO `leitzprint.pkg`. Examined actual Mach-O `raster2printerV3` and `command2leitzV3`, as well as vendor PPD `1.2.190`.
- macOS PPD exposes 41 named pages (including `Continuous112` not present under that key in Windows GPD); Windows has 76 named pages, including many Leitz-prefixed aliases. **Do not conflate these source catalogs or assume all geometries match.**
- Windows continuous GPD page height: 7680 pt; macOS continuous PPD page height: ~7654 pt. The discrepancy must be preserved until the differing raster semantics have been established.

## Linux v0.4.1 feature parity matrix

| Capability | Linux evidence | Vendor capability / remaining task |
| --- | --- | --- |
| USB `289e:1000`, `ESC U` material query | **Physically verified** with 88 mm white paper, 88 mm yellow plastic | Extend material and error status beyond SKU |
| 300×600 and 300×300 dpi | **Both physically verified** on white paper | Other widths and cassette SKUs not verified |
| Fixed page cut / multi-page print | **Physically verified** with white roll at 100 mm | Determine die-cut mechanics and original media command semantics |
| Custom size / last-ink auto-cut | **Physically verified** with white roll | The yellow cassette printed 49–51 mm when 60 mm was requested, and the 100-mm yellow test stalled after part of its first printed row. Root causes **not established** |
| Vendor media names | Original macOS human-readable names reviewed; 76 options relabeled **in development branch**, keeping their technical keys and geometry | Validate missing/different media definitions against the two source systems and actual hardware |
| Printer status during print | **Not implemented** in Linux backend | macOS `raster2printerV3` calls `ESC A` and parses returned status codes. Vendor states include busy, door open, paper jam, head overheat, cutter jam, no cartridge |
| Device-side print progress | **Not implemented**; Linux CUPS may report idle even after incomplete physical print | macOS `raster2printerV3` issues `ESC P`, reads the number of printed lines, and has a resume path. The exact response framing and safe retry behavior require investigation |
| CUPS command channel | **Not implemented** | macOS `command2leitzV3` exposes GetPrinterStatus, GetUnitedStatus, GetFirmwareVersion, GetWiFiStatus and ReportStatus; do not add PPD `cupsCommands` until a working Linux command filter exists |
| Print quality | Linux accepts one-bit CUPS raster | macOS PPD offers Draft plus several dither/halftone algorithms (Bayer, SuperCell and others). Requires filter integration, not merely PPD options |
| Cutter controls | Linux exposes Automatic, Page and Content, always emits end-of-job cut | macOS PPD has CutMedia and LabelCutter options, including disabling per-label cut; actual command semantics must be mapped |
| 64-byte boundary | Linux **only warns** if job size is a multiple of 64 | Windows LeitzRp.DLL includes a *conditional* code path that sends an additional ESC byte. The precise predicate and correct insertion point must be established from the renderer; do not blindly append bytes |

## Architectural correction

The Linux backend currently validates and transmits a complete binary job via one `cat` write, then returns success when bytes have been accepted by the kernel. That is not equivalent to confirmed physical completion.

Finish vendor protocol research and implement an explicit state machine for **material preflight → send → live status/progress → confirmed completion or unambiguous failure**. Investigate recovery without automatic blind retransmission (duplicate labels/cuts can result).

## Validation policy

No more one-change/one-label iteration. Use offline tests for protocol frames, PPD, dithering, media layouts, error handling, and mock USB responses. Schedule a **single batched physical test session** only after the related changes are integrated. Preserve exact original Windows and macOS terminology in the test comparisons.

**Development branch only:** `port/vendor-parity`. The TARS installation and `main` remain unchanged until the integrated implementation is verified.
