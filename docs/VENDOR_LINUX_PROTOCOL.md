# Native Linux 1.0.3 protocol comparison

**Basis:** `LeitzIconDriversForLinux1.0.3.zip`, the manufacturer-published 2014 Linux archive, examined locally. This document describes observed source-code behavior. It does not certify the complete hardware protocol or a physical test of our new encoder.

**Original source reviewed:** `raster2printer.c`, `raster2printer.h`, `raster_c_functions.c`, `raster_c_functions.h`, and `PPD/LeitzIcon.ppd.gz`. Do not commit the manufacturer's copyrighted source to this repository.

## Shared page and cut controller in our development driver

The original Linux 1.0.3 implementation is now **one selectable wire codec**.
The internal `leitz_job.c` module owns shared multi-page sequencing and
`LabelCutter=ON/OFF` policy for both `Current` and `NativeLinux103`.
The `leitz_cups_filter.c` raster path is no longer protocol-specific.
Only the command/raster encoding and its exact end-of-job bytes differ.

The comparison below describes **historical v0.4.1 bytes** and original Linux
source, not separate feature sets in our newly refactored job controller.

## Observed framing, not an assumed specification

| Step | Native Linux 1.0.3 (source) | Our tested v0.4.1 |
| --- | --- | --- |
| Job start | `ESC @` then `ESC i` (600 dpi) or `ESC h` (300 dpi), before first page | 120 `ESC` preamble then reset/resolution embedded in each page header |
| Page header | 120 `ESC`, `ESC L` (2-byte length), `ESC w` (2-byte width), `ESC B 0` | 120 `ESC`, `ESC @`, `ESC B 0`, `ESC L`, resolution, `ESC w` |
| Raster line (optimization level 1) | `ESC D` (byte width), `SYN 0x16`, packed monochrome raster | `ESC D` on width change; `ESC B` (offset), `0x16` + packed raw or `0x17` + RLE |
| Blank lines | `ESC f` + 16-bit count | `ESC f` + 16-bit count |
| End of final document | `ESC G ESC E` | `ESC G ESC E ESC @ ESC Z` |
| In-between pages | `ESC G ESC E` if `LabelCutter=ON`; `ESC G` if OFF | `ESC G ESC E ESC @ ESC Z` after **every** page |
| 64-byte multiple | No visible special padding in the 2014 Linux raster source | A warning in the USB backend; no padding |
| USB/status | The original Linux raster filter writes its bytes to stdout and does not itself issue `ESC U` or confirm physical completion | Own USB backend queries cartridge using `ESC U`, but currently confirms only accepted writes |

**Important:** the native Linux original itself is not proof that every feature is correct or complete. In particular its `LabelCutter=OFF` choice still emits `ESC G ESC E` on the final page.

### Safe reference implementation

`src/leitz_native_reference.[ch]` independently reconstructs the **level-1 plain/raw** command framing above. It intentionally does **not** implement vendor source algorithms for modes 0/2/3, CUPS profile selection, status reads, USB writes or firmware recovery. It accepts already-packed monochrome rows; all-blank rows **before** a printed line are combined into `ESC f` commands. To reproduce a quirk of the source, **trailing** skipped blanks are not flushed at the end of a page.

Run `make native-reference-test` to verify frame bytes, 300/600 dpi, blank line aggregation (including original unflushed trailing blanks), end marker, malformed data and explicit offline selection by `leitz-cli validate-native`. The native implementation is now available within the **development** CUPS raster filter as explicit `LeitzProtocol=NativeLinux103`, while the known-good stream remains the default. `leitzusb` validates the selected stream independently before any USB transfer. Physical testing remains pending.

## Remaining porting work (grouped)

1. **Implemented in development:** `LeitzProtocol=NativeLinux103` selects the original Linux framing in the real CUPS raster filter. The backend detects and validates either stream before opening USB, including distinct ESC-w width offsets. Current remains the default. Remaining: compile and verify on TARS; test hardware output before choosing the final implementation. Native now supports multiple pages with the same width/resolution and `LabelCutter=ON/OFF`, exactly one reset at document start and final `ESC E` in either mode.
2. Reconcile raster polarity, CUPS margins, length units and media geometry across the Windows GPD, original Linux PPD and macOS PPD without confusing catalog keys with device-cartridge SKUs.
3. **Implemented in development:** common `LabelCutter=ON/OFF` policy in the CUPS print pipeline for both `Current` and `NativeLinux103`, including dialect-specific page terminators and full-document validation. Physical behavior is pending a combined hardware test.
4. **Implemented:** known ready byte `0x03` is checked via `ESC A` before USB print data. **Remaining:** during-print progress and completion handling once supported by actual device responses; `ESC P` timed out at idle. The macOS driver is a separate source; its behavior cannot be inferred from this Linux package.
5. Batch physical testing on TARS for different cassettes and both resolutions; never infer physical completion from CUPS `idle`.

**Hardware test not yet completed.** Use the native experimental option only during a controlled test; the current verified stream is the default.
