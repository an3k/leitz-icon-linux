# Windows LeitzRp 2.17.4.112 — partial portable C reconstruction

**Scope:** The original x64 raster path at relative virtual addresses
0x26df–0x274b and 0x2907–0x29a7 of the manufacturer's `LeitzRp.dll`.
A standalone GCC/Clang test verifies output bytes and the 16-bit
previous-width state. The Windows COM ABI is **not** claimed.

Run `sh test.sh`. No Windows executable runs, no hardware is accessed.

Not yet reconstructed: compressed-row encoder, final-byte source clipping,
blank-line skipping (`ESC f`), job/cut state (`ESC G`), dynamic
label-length and ModeAuto/IsDieCut. This code does not replace the active
Linux CUPS filter. See `analysis/windows-2018/RECONSTRUCTION_STATUS.md`.
