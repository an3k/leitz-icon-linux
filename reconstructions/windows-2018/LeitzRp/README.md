# Windows LeitzRp 2.17.4.112 — partial portable C reconstruction

**Scope:** The original x64 raster path at relative virtual addresses
0x26df–0x274b and 0x2907–0x29a7 of the manufacturer's `LeitzRp.dll`.
A standalone GCC/Clang test verifies output bytes and the 16-bit
previous-width state. The Windows COM ABI is **not** claimed.

Run `sh test.sh`. No Windows executable runs, no hardware is accessed.

The independent row_control.c module now reconstructs the OEM's
per-row ESC w / conditional ESC E / queued-blank ESC f / label-end ESC G
transition at x64 RVA 0x2060–0x237a. The original state offsets
+0x08/+0x0a/+0x0e/+0x10/+0x12/+0x18/+0x1c are documented
individually; its C struct is a new portable interface, not the native
C++ object. The row_index is managed by the original's outside caller.
An encoder callback represents the still-unrecovered full compressor.

The original x64 and x86 RLE body is now implemented in the independent
`rle_windows_2018.[ch]` module. It accepts already-trimmed byte rows,
retains 1..128-bit black/white runs, and returns USE_RAW unless output
is strictly shorter than input. The optional last-byte copy-and-mask
helper matches the manufacturer's conditional physical-width masking
without modifying caller-owned source memory. Test vectors are checked
against an independent bit-by-bit reference and the original
disassembly; this is **not direct execution parity against Windows**.

Not yet reconstructed: full automatic label-length control,
original COM object and Windows shell integration. The core RLE and
safe final-byte source clipping are separate new C test interfaces,
not copies of original OEM function signatures.

Remaining: caller-owned row index lifecycle, custom-length ModeAuto and
IsDieCut decisions, and original COM entry points. None of these
experimental fragments replaces the Linux CUPS driver. This code does not replace the active
Linux CUPS filter. See `analysis/windows-2018/RECONSTRUCTION_STATUS.md`.
