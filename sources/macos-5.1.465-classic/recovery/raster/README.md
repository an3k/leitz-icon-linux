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


## Recovered `_ReadPage` and `_WriteData` branch fragments

This revision adds two newly authored, isolated C interfaces corresponding to **observed manufacturer branches**. Neither is a complete clone of the original Mach-O function, argument ABI or surrounding macOS/CUPS integration.

**`classic_read_page.[ch]`** — original `_ReadPage`, `0x100008d20..0x10000918b`:

- `cupsRasterReadHeader2` returns zero: EOF; nonzero: page counter increments *before* the pixel allocation or reading.
- 32-bit `cupsHeight * cupsBytesPerLine` allocation expression and one `cupsRasterReadPixels` request for the entire page.
- A zero returned pixel count selects the failure branch. A **nonzero short read is accepted**, reproducing the branch observed in assembly rather than the protective complete-read check in the enhanced implementation.
- The native page buffer layout, all header-dependent state and original ownership/error unwinding are **not** recovered. The standalone C callback interface therefore exposes allocation ownership explicitly; this interface itself is not manufacturer code.

**`classic_write_data.[ch]`** — original `_WriteData` at `0x100001450`:

- One selected `send` call for IPP/socket output; a positive short send is not retried.
- The alternate stdout path optionally writes to the PRN/debug file before stdout; the debug write return is not treated as fatal.
- The offline interface is explicitly passed a previously selected sink: **the OEM's full URI/global-file selection is not yet recovered**, and its original return-value ABI and diagnostic side effects have not been verified.

Source-only checks `classic_read_page_test.c` and `classic_write_data_test.c` cover these recovered decisions and are included in `make check`. The original programs have **not** been executed for comparison and there is **no 1:1 equivalence claim**.

## _ProcessPage modes 0/1 — extracted raster output framing

`classic_process_page_modes_01.[ch]` reconstructs the previously audited **mode 0** and **mode 1** row-submission path of the original `_ProcessPage`, observed at `0x100007633..0x100007739`, `0x100007939..0x100007a74`, `0x100007e6e..0x100007f32` and `0x10000889b..0x100008945`.

- Both modes emit one four-byte `ESC D <packed_stride> SYN` introduction, separately from the page setup.
- Mode 0 sends every complete packed row unchanged.
- Mode 1 accumulates all-zero rows and emits `ESC f <two-byte count>` before the next nonempty row; trailing empty rows remain deferred.
- The OEM length is based on each job's raster row, without any general fixed paper length. The source's 16-bit blank-row counter is based on the disassembly; what happens on extremely long overflows still needs execution against the original.

These are standalone fragments, **not** complete `_ProcessPage`: mode 2/3 branches, state carried between pages, cancellation, complete page positioning and error paths remain missing in this module. The previous enhanced full-row approximation is still under `enhancements/`. A separate test checks framing and deferred blank rows. The original Mach-O has not been run.

## _ProcessPage modes 2/3 — isolated row emission

The new `classic_process_page_modes_23.[ch]` implements the previously audited, disassembly-derived **row-level** mechanics of mode 2 and mode 3, with the original byte framing and state distinctions:

- Both accumulate empty rows as deferred 16-bit `ESC f` advances, emitted only when another nonempty row occurs.
- Mode 2 always emits the `ESC B` byte offset, trims both byte edges, emits an `ESC D` only on changed packed width (or the first nonblank row of an explicit reprint invocation), and selects `0x17` bit-run encoding **only when strictly shorter** than `0x16` plus raw trimmed row bytes. Run bytes encode 1–128 bits, most-significant-bit first.
- Mode 3 emits `ESC B` only when its left offset changes; the original mode-3 path trims the right-hand white bytes **only if the left offset is nonzero**. It always emits uncompressed `0x16` raster bytes.
- Mode-2 and mode-3 previous widths survive page boundaries in different job-cache fields. The previous mode-3 left offset, pending blank rows and first-nonblank state are reset at each page start.
- The compiler-facing state structures are **newly authored analysis interfaces**, not reconstructed original structure layouts. Caller-provided output capacity is required; this file deliberately does **not** inherit the bounds/error-handling improvements of the separate `enhancements/` version.

`classic_process_page_modes_23_test.c` covers the observed compression case, raw fallback, original mode-3 trimming asymmetry, blank feed, cached width across page boundaries, and the explicit reprint branch.

This module does **not** restore the entirety of OEM `_ProcessPage`: the original's loop-level progress/cancellation order, all memory accesses, option reading, start/end integration, all error paths, and original ABI are not covered. Its tests compare with previously audited reconstruction expectations; **the manufacturer Mach-O is not executed**. Neither functional identity nor full branch coverage may be claimed.
