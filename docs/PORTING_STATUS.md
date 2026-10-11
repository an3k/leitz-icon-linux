# Leitz Icon Linux — vendor porting baseline (10 Oct 2026)

This is an **evidence-based gap inventory**, not a claim of full parity. The original vendor binaries remain private and are not committed.

## Independent vendor sources examined

- Windows 2018: `LeitzIcon.gpd` (76 PaperSize entries), `LeitzRp.DLL` (OEM raster rendering), `LeitzLm.dll` (language/status monitor), `LeitzUI.DLL`.
- macOS installer 5.1.465: extracted the Apple HFS+ DMG, nested XAR distribution and gzip/ODC-CPIO `leitzprint.pkg`. Examined actual Mach-O `raster2printerV3` and `command2leitzV3`, as well as vendor PPD `1.2.190`.
- Original Linux 1.0.3 has **40** named PPD media, macOS 1.2.190 has **41** (adding `DHLShippingLabel`); the Windows GPD contains **76 options**, including `CUSTOMSIZE` with no fixed dimensions, leaving **75 concrete Windows sizes**. The union is 81 named media sizes plus the Windows `CUSTOMSIZE` setting. **Do not conflate these source catalogs or assume all geometries match.**
- Windows continuous GPD page height: 7680 pt; macOS continuous PPD page height: ~7654 pt. The discrepancy must be preserved until the differing raster semantics have been established.

## Linux v0.4.1 and development-branch feature parity matrix

| Capability | Linux evidence | Vendor capability / remaining task |
| --- | --- | --- |
| USB `289e:1000`, `ESC U` material query | **Physically verified** with 88 mm white paper, 88 mm yellow plastic | Extend material and error status beyond SKU |
| 300×600 and 300×300 dpi | **Both physically verified** on white paper | Other widths and cassette SKUs not verified |
| Fixed page cut / multi-page print | **Physically verified** with white roll at 100 mm | Determine die-cut mechanics and original media command semantics |
| Custom size / last-ink auto-cut | **Physically verified** with white roll | The yellow cassette printed 49–51 mm when 60 mm was requested, and the 100-mm yellow test stalled after part of its first printed row. Root causes **not established** |
| Vendor media names | **86 PPD entries in `port/vendor-parity`: 43 alphabetical A profiles followed by 43 `Leitz`-prefixed B profiles**. The 39 existing Windows B labels come from the original resource strings; four additional B labels are derived from A. All 81 existing profile geometries are unchanged | Four original A/B geometry differences are intentionally preserved: `LargePendaflexTabRotated`, `Multipurpose70030001Rotated`, `NameBadgeCard70190001Rotated`, `ShippingLabel70180001Rotated`. New aliases are not physically verified |
| Printer status | **Implemented preflight in development:** after ESC U cartridge verification, ESC A must return the physically observed `0x03` READY byte before USB print transfer | Live during-print monitoring still not implemented; mechanical completion remains unverified |
| Device-side print progress | **Not implemented**; Linux CUPS may report idle even after incomplete physical print | macOS `raster2printerV3` issues `ESC P`, reads the number of printed lines, and has a resume path. The exact response framing and safe retry behavior require investigation |
| CUPS command channel | **Implemented in development for `ReportStatus`:** a bounded CUPS maintenance-file filter produces an internal control frame; `leitzusb` queries `ESC A` exclusively and reports only legacy status codes conservatively | Other macOS status/firmware/WiFi commands are not exposed through this channel; the new CUPS path has passed CI but no direct hardware test |
| Print quality | **Implemented in development:** existing one-bit CUPS raster plus optional 8-bit DeviceK/DeviceGray conversion with selectable `LeitzDither=Bayer4/Threshold`, shared by both codecs; offline raster regression tests pass | Original macOS Draft/SuperCell/Bayer variants are not byte-for-byte ported or physically benchmarked |
| Cutter controls | **Shared `LabelCutter` policy in development:** `ON` requests a cut after every label; `OFF` requests one at job end. Both `Current` and `NativeLinux103` encoders serialize the same decision in their own framing. The previous `Current` default byte stream remains unchanged. | The new `Current` no-cut-between-pages sequence and native cutter behavior are not yet hardware-verified; macOS-specific semantics remain open |
| 64-byte boundary | The current development filter/backend **neither pads nor applies a validated 64-byte correction** | Windows LeitzRp.DLL includes a *conditional* code path that sends an additional ESC byte. The precise predicate and correct insertion point must be established from the renderer; do not blindly append bytes |

## Original Linux source comparison

- The manufacturer-published native Linux 1.0.3 source (`raster2printer.c`, `raster_c_functions.c`) sets job defaults with `ESC @` and resolution `ESC h`/`ESC i` before each page header. Each page begins with 120 `ESC` bytes and includes `ESC L`, `ESC w`, `ESC B 0`. Raster rows use `ESC D`, `SYN` and bytes; optimization level 2 can trim white bytes. The final page uses `ESC G ESC E`; intervening pages omit `ESC E` when `LabelCutter=OFF`. These differ materially from the current Linux v0.4.1 encoder and require a separately validated implementation.
- Neither that original Linux raster filter nor our current USB backend verifies physical completion. The manufacturer source exposes `OptimizationLevel`, `ContinuousMedia`, `ContinuousPaperLength`, `LabelCutter`, `ShouldCorrectMargins`; options in the PPD are not all necessarily implemented in the filter.
- The proprietary vendor C source and binary packages are **not committed** to this public repository. Derived media metadata is kept separately in `tools/source_media_additions.json`.

## Architectural correction

The Linux backend currently validates and transmits a complete binary job via one `cat` write, then returns success when bytes have been accepted by the kernel. That is not equivalent to confirmed physical completion.

Finish vendor protocol research and implement an explicit state machine for **material preflight → send → live status/progress → confirmed completion or unambiguous failure**. Investigate recovery without automatic blind retransmission (duplicate labels/cuts can result).

## Validation policy

No more one-change/one-label iteration. Use offline tests for protocol frames, PPD, dithering, media layouts, error handling, and mock USB responses. Schedule a **single batched physical test session** only after the related changes are integrated. Preserve exact original Windows and macOS terminology in the test comparisons.

**Development branch only:** `port/vendor-parity`. The TARS installation and `main` remain unchanged until the integrated implementation is verified.

## Native Linux 1.0.3 framing integrated into driver (development only)

**What changed:** `leitz-rastertoicon` now accepts `-o LeitzProtocol=NativeLinux103`, and the PPD exposes `LeitzProtocol` with **Current** as default and **Original Linux 1.0.3 (experimental)** as the opt-in choice. Both variants use the **same** CUPS raster input, page geometry, media selection, cut-length calculation and staged output. The final encoder is selected inside the same running CUPS filter: the old `leitz_codec` remains the tested default, while the native Linux 1.0.3 raw framing is produced by `leitz_native_reference`. The backend detects native first bytes (`ESC @ ESC i/h`), runs the complete **native validator**, checks width at its distinct header offset, then performs the **same** SKU/width/USB transfer checks. Nothing reaches USB before whole-job validation.

**Current native restrictions:** NativeLinux103 supports multiple labels with the same raster width and resolution. The independent vendor-reference codec still omits unflushed trailing blank rows, but the real CUPS job controller explicitly flushes those rows for `ESC L=0xffff` continuous/custom pages. Fixed-media source behavior is unchanged. This correction has passed offline tests, not a physical length measurement. `ESC P` and physical completion remain unverified. The native dialect has **not** been tested on actual TARS hardware. The separate offline reference source and its tools remain present; nothing was deleted. The goal is to converge on one verified final encoding after hardware comparison.

**Tests:** The existing `tests/test_v04.py` now exercises the real raster-filter native option at 300×300 and 300×600 dpi in addition to the unmodified Current path. The development-branch Docker image build has passed GitHub Actions; the integrated native-protocol live printer test remains pending.

## Native Linux 1.0.3 wire-reference implementation — offline only

- Added independent C reference encoder `src/leitz_native_reference.[ch]` and strict single-label validator `src/leitz_native_validate.c` for the **plain/raw optimization-level-1 subset** of the actual manufacturer Linux 1.0.3 C sources. These are not vendor source copies.
- Added offline C regression tests and `make native-reference-test`; checked with local GCC at both 300×300 and 300×600 dpi, against explicit byte-frame fixtures, rejection cases and truncated/altered end markers.
- The reference reproduces the native source's **unflushed trailing blank rows**. This behavior is not automatically a desirable choice for continuous media; physical label length is not inferred from the emitted row count.
- **Reference defect corrected:** its former unconditional end-of-label blank-row flush contradicted the original Linux 1.0.3 optimization-1 source and pre-existing regression fixtures. The encoder now leaves trailing blank rows pending (the productively tested v0.4.1 encoder is unchanged).
- **Offline validator integrated:** `leitz-cli validate-native file.bin` now checks the original Linux 1.0.3 framing separately, without changing the existing `leitz-cli validate` dialect. `make native-reference-test` additionally verifies that native frames are rejected by the production validator and tests truncation/extra data. Now supported by the development-branch CUPS filter and backend, with full-document multi-page native validation; **not yet physically verified**.
- Exact framing comparison: [VENDOR_LINUX_PROTOCOL.md](VENDOR_LINUX_PROTOCOL.md).

## Architekturkorrektur: gemeinsame Auftragssteuerung (10.10.2026)

Die beiden Projektquellen zur IST- und Zielarchitektur legen die Trennung fest:
`leitz_cups_filter.c` verwaltet **Medien, Raster und Druckoptionen**, die neue
`leitz_job.c` als internes Modul **Mehrseitenablauf, gemeinsamen Vorschubaufruf
und Schnittentscheidung**. Die Auswahl `Current`/`NativeLinux103` erfolgt
erst in `leitz_job.c` beim Aufruf der beiden Byte-Encoder
`leitz_codec.c`/`leitz_native_reference.c`. Codec-spezifische Bytes für den
letzten Schnitt bleiben dort gekapselt. `leitzusb`, `usblp`, der PPD-Katalog
und der PDF-Konverter sind unverändert.

Die Schnittoption `LabelCutter=OFF` gehört jetzt **nicht mehr exklusiv zum
alten Linux-Protokoll**; beide Encoder werden durch dieselbe
Auftragssteuerung behandelt. Bei einem einzelnen Etikett ist der resultierende
Datenstrom identisch, unabhängig davon, ob `LabelCutter=ON` oder `OFF`
gewählt wurde. Die ursprünglichen Einzelblatt-/Standardkodierfunktionen
bleiben erhalten; nur ihre übergeordnete Ansteuerung wurde neu geordnet.
Ein abschließender physischer Test ist noch ausstehend, die Änderung läuft
ausschließlich auf `port/vendor-parity`.

## Integration since the reference-only stage (development branch)

- The real CUPS filter now emits **multi-page native Linux 1.0.3** documents with a single reset/resolution at job start and a page header for each label. **Shared CUPS job policy** selects `LabelCutter=ON/OFF` for both wire dialects. Each encoder translates the same cut decision into its framing; `Current` continues to produce identical bytes by default. The newly enabled `Current` no-cut-between-pages path is experimental until physically tested.
- `leitz-cli validate-native` now validates all pages and their boundaries, rejects mixed widths/truncated final cuts, and is mandatory for native data at the USB backend. The backend's existing full-job spooling/SKU/width/USB checks remain.
- The real backend now issues `ESC A` immediately before the first print byte, after verifying the cartridge, and permits transfer only for the physically verified ready byte `0x03`. This is a **pre-send** check, not monitoring during printing and not a mechanical-completion proof.
- All changes remain on `port/vendor-parity`. The user requested **one combined hardware test later**, not one Docker build/label after each edit. Existing test tools are retained.

## Real idle status probes (10 October 2026)

See [controlled status capture](CONTROLLED_STATUS_CAPTURE.md). `ESC A` returned one byte `03` (ready per original macOS decoder); `ESC V` returned a 34-byte NUL-terminated ASCII record with fields `ID`, `A`, `M`, `W` (device ID redacted from the repository). `ESC P` at idle returned **zero bytes and timed out after seven seconds**. Support during active printing and safe print-job recovery remain **unverified**. No automated progress/resume functionality is enabled. The diagnostic now formats every successful reply as both `REPLY_HEX` and decoded content from the shared offline parser; no USB protocol or CUPS behavior is altered.

## 43 + 43 alias catalog (development branch)

The branch PPD, `media.json`, and `src/leitz_media.h` now contain 86 entries in **A followed by B** order. Newly introduced names: `LeverArchFile1005` in A; `LeitzContinuous112`, `LeitzDHLShippingLabel`, `LeitzNameBadgeCard8214`, `LeitzWristband` in B. `tools/media_b_original_labels.json` records which B names are verbatim Windows originals versus explicit newly derived aliases. `make media-check` performs the read-only structural integrity gate; `make mock-filter` then `python3 tests/test_v04.py` is the separate offline raster test, which is not a physical device test. The PPD on TARS is unchanged.
