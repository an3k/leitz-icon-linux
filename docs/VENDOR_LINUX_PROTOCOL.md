# Native Linux 1.0.3 protocol comparison

**Basis:** `LeitzIconDriversForLinux1.0.3.zip`, the manufacturer-published 2014 Linux archive, examined locally. This document describes observed source-code behavior. It does not certify the complete hardware protocol or a physical test of our new encoder.

**Original source reviewed:** `raster2printer.c`, `raster2printer.h`, `raster_c_functions.c`, `raster_c_functions.h`, and `PPD/LeitzIcon.ppd.gz`. Do not commit the manufacturer's copyrighted source to this repository.

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

Run `make native-reference-test` to verify frame bytes, 300/600 dpi, blank line aggregation (including original unflushed trailing blanks), end marker, malformed data and explicit offline selection by `leitz-cli validate-native`. This is fully offline; the production `leitz-rastertoicon` and `leitzusb` are **not modified or switched**.

## Remaining porting work (grouped)

1. Offline dialect validation now exists explicitly: `leitz-cli validate` checks the tested v0.4.1 stream, `leitz-cli validate-native` checks the original Linux 1.0.3 reference stream. **Remaining:** create a genuinely selectable print pipeline and only then allow the selected dialect through `leitzusb` after corresponding validation; native is not currently sent to USB.
2. Reconcile raster polarity, CUPS margins, length units and media geometry across the Windows GPD, original Linux PPD and macOS PPD without confusing catalog keys with device-cartridge SKUs.
3. Add documented cutter options and per-page semantics (including no-cut-between-pages) and offline tests.
4. Implement real status/progress handling only after confirming wire command response framing and retry safety. The macOS driver is a separate source; its behavior cannot be inferred from this Linux package.
5. Batch physical testing on TARS for different cassettes and both resolutions; never infer physical completion from CUPS `idle`.

**Do not use this reference encoder for printing until that integration and validation are complete.**
