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
An encoder callback is preserved in the standalone row controller; the RLE
codec itself is independently recovered and tested below.

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

The combined `rle_pipeline_test.c` exercises the existing prefix writer,
full RLE byte run computation and 0x16/0x17 payload selector together,
including byte-level output commands and their separate callback calls.
No production Linux code or COM entry point is changed.


The x64 OEM start-of-job media/length initializer RVA 0x16f7–0x1c4e
has now been reconstructed in label_setup.[ch]. It initializes the
already-existing row controller without reimplementing its output
functions, reading original paper-name flags ("Continuous" and
"CUSTOMSIZE"), ModeAuto, iLabelLength, dmOrientation and the GDI
raster-width/raster-height pair. It reproduces the conditional
landscape swap, diecut/custom-size flags, configured-versus-
automatic length and 16-bit native state truncation. No fixed length
cap is introduced. The original OEM +0x18 flag used by ESC w is
now positively identified as IsCustomSize. Its original C++ ABI
and all run-time Windows APIs are not duplicated or claimed.

Integration of label setup with previously recovered row control: `label_setup_row_test.c` checks that original-derived
`CUSTOMSIZE` enables per-row ESC w and terminates at raster height;
continuous manual-length sends ESC f/G at the provided iLabelLength;
continuous automatic-length terminates at raster height instead.
No encoder or USB backend changes are involved.


## Complete counted-row stream (offline adapter)

The standalone `raster_stream.[ch]` composes the previously
reconstructed media initialization, original-derived row state, prefix
width/margin commands, bit RLE encoder and raw/compressed selector into
one bounded C callback stream. It does not rewrite those source modules.
Its encoder uses caller-owned, separate scratch buffers and masks unused
final bits only in a copy. A per-row diagnostic captures an ignored
FilterGraphics result without changing the native-inspired row return.

`raster_stream_test.c` covers two consecutive synthetic labels,
raw versus RLE paths, blank-run feeding, medium switches, cache reset,
manual continuous length, conditional final-bit clipping, separate
WriteData call boundaries and failure/buffer behavior.

The vectors are derived from already documented OEM fragments; **neither
the Windows original nor a physical printer was executed**. This is not
the Windows COM/Unidrv caller, and the physical USB backend, native
exception semantics and OEM-vs-reconstruction parity remain open.
Run `sh test.sh` for GCC and Clang checks.
