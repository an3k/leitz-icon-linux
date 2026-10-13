# Classic macOS 5.1.465: independently recovered raster job boundaries

Reference binary: `raster2printer`, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.

The C implementation corresponds to specific disassembled functions, not the entire manufacturer executable:

- `_StartDocument` (`0x100006890`): 120 leading ESC padding bytes followed by ESC @, optionally ESC i for 600 dpi.
- `_StartPage` (`0x100007380`): ESC L with job-derived height, ESC w with raster width, ESC B 0.
- `_EndPage` (`0x1000089b0`): ESC G and optionally ESC E; original calls `_FlushDataStreams` afterwards.
- `_EndDocument` (`0x100008ac0`): optional ESC E, always ESC @ and ESC Z. Original also closes the CUPS raster stream.
- `_FlushDataStreams` (`0x1000015f0`): `fflush(stdout)` and optionally `fflush(__fd_dump)`; its return values are ignored by the manufacturer executable.

**Build check:** `make clean all check` succeeded with GCC 12, `-std=c11 -Wall -Wextra -Werror`. This checks these isolated output decisions, not a complete print job.

**Still missing:** faithful integration of `_raster2printer`, `_ReadPage`, `_DitherPage`, the complete `_ProcessPage` state machine, network/PRN output, and the remaining functions and error/termination paths. The existing experimental work under `enhancements/` is kept separate because it contains deliberate changes to the original behavior.

The macOS binary has not been executed and compared for exact output. This is not a completed 1:1 reconstruction.
## Original halftone tables and PrintQuality selector — 2026-10-13

- `raster2printer_matrices_recovered.c`: preserves all **13 sets of integer matrix values** previously extracted from the vendor Mach-O data segment; the original addresses and previously recorded raw checksums remain in the array comments. All **5237** entries are transferred as data, without any changed matrix values.
- `raster2printer_quality_recovered.c`: preserves the **15 choice spellings and selector indices** previously established for original `_raster2printer` code at `0x100004dbd..0x100005098`. All 15 choice names were checked against the genuine classic PPD. Case-insensitive comparisons match the observed choice comparisons. In particular `CUPSThresold64x64` keeps its original spelling, and `Draft` is index 7.
- Selector descriptors preserve 15 original table/dimension/divisor entries, including the two references to the same 144-element matrix for modes 1 and 2. For `Draft`, the null table is a documentation sentinel: the observed binary reaches that path without an initialized matrix pointer; `Draft` uses 1-bit input in the original PPD, not RGB dithering.
- `raster2printer_quality_recovered_test.c` and the updated Makefile describe offline **compilation/check targets**. No fresh compiler execution or original Mach-O comparison has been performed in the current session; the local execution environment was unavailable. Previously recorded smoke tests are not evidence for this new revision.

These are recovered **data and option-selection fragments**. They are not a recovered implementation of the original `_DitherPage`, nor are they a drop-in binary replacement. They contain no deliberate alterations to the 13 original numeric arrays. The original Mach-O files themselves were not stored in this GitHub branch.

## Recovered _DitherPage RGB24 pixel conversion

`raster2printer_dither_recovered.[ch]` now records the **previously reverse-engineered** inner RGB24 conversion from OEM `_DitherPage` (Mach-O `0x100006dd0..0x100007338`). It uses the OEM 13 matrix tables, choice indices, floating-point divisors and bit packing without the added validation of the prior experimental code. A minimal valid-RGB24 input test is included in the existing offline Makefile.

This is a **C fragment**, not the OEM function ABI: the surrounding original memory allocation, raster header, per-page option selection, fallback and error paths are **not recovered**. The OEM `Draft` choice selects a 1-bit input route; calling this RGB24 fragment with selector 7 is outside the recovered path. The experimental source with deliberate input validation stays under `enhancements/`.

No fresh compilation, binary-reference execution or printer test was performed in this session: the local compiler/container runtime is unavailable. The prior reconstructed model is not a substitute for byte-exact comparison with the original macOS executable.
