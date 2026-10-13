# macOS 5.1.465 classic — reconstruction state

## Manufacturer inputs (unmodified)

- `raster2printer`: Mach-O x86_64, 84,252 bytes, SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
- `command2leitz`: Mach-O x86_64, 35,932 bytes, SHA-256 `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee`.
- `LeitzIcon.ppd`: unmodified manufacturer PPD.
- The original archive does **not** contain original C files for either executable.

## C source availability

**Neither executable has a complete faithful C reconstruction.** The existing symbol lists and assembly analyses describe specific functions but cannot establish the original C source, all code paths, or full binary-to-C equivalence.

Experimental, compilable C fragments formerly kept in `reconstructed/` have been moved unchanged into `enhancements/macos-5.1.465-classic/reconstructed/`. Do not attribute these files to the manufacturer or use them as exact replacements.

The fragment library, command filter and test executables were built using GCC on Linux (`make clean all check`, successful); the full CUPS raster adapter could not be compiled in the local environment because genuine CUPS development headers are unavailable. The original Mach-O executables have not been run for reference-output comparison.

## Documented intentional divergences in the experiments

- Rejecting stdout/PRN flush errors where the original routine does not check the return code.
- Extending the portable raster-header callback to distinguish negative errors from end-of-file.

Further differences may exist. Neither the successful builds nor local smoke tests establish equivalent printing or cutting behavior.

**Before a 1:1 claim:** reconstruct every active code path for both executables; compile for macOS using compatible system APIs; compare identical inputs/options, device-output bytes, status exchanges, errors and edge conditions against the original executable. No such final comparison is available.

## Additional recovery committed on 2026-10-13

- `recovery/command2leitz_recovered.c` plus `command2leitz_cups.c`: independently recovered active command-dispatch and CUPS entry path, built and exercised offline. Original execution and full macOS equivalence remain outstanding.
- `recovery/raster/`: five raster lifecycle/output boundary functions reconstructed separately from assembly; built and checked offline. The raster filter as a whole remains incomplete.
- Existing experimental C stays isolated under `enhancements/`, and the manufacturer originals remain unmodified.

## 2026-10-13 — recovered manufacturer halftone values

- All 13 matrix tables and all 5237 integer elements previously extracted from the classic Mach-O are now present in standalone C under `recovery/raster/`. C declarations expose these arrays without changing their numeric values.
- All 15 original `PrintQuality` labels were matched against the byte-identical original PPD, and their previously reverse-engineered selector indices, matrix sizes and scaling constants are in separate C.
- Full `_DitherPage` and full raster filter remain incomplete; no 1:1 equivalence claim. This revision has **not** been newly compiled because the container runtime was unavailable.

## 2026-10-13 — dither inner loop reconstruction

Added `recovery/raster/raster2printer_dither_recovered.[ch]`, extracted from the already documented OEM `_DitherPage` RGB24 arithmetic. The recovered fragment uses manufacturer matrix values and exact previously decoded mode selection, without previous experimental boundary checks; additional test inputs have been added to the offline Makefile. **The complete original function is not recovered, and no compiler or OEM execution check was possible in the current session.**

## 2026-10-13 — raster input and output recovery

New standalone C in `recovery/raster/`: `classic_read_page.[ch]` and `classic_write_data.[ch]` with explicit callback boundaries. These correct two differences from previously hardened project code: page index increments at header acceptance and nonzero short page reads remain successful; short socket sends are not automatically retried. Original branching evidence is documented by original Mach-O virtual addresses. No full ABI, complete job path or binary-equality assertion is made. GCC/Clang CI build verification is configured separately and must not be reported as passing until the workflow concludes successfully.

## 2026-10-13 — ProcessPage modes 0/1 independent recovery

Added `recovery/raster/classic_process_page_modes_01.[ch]`: original 0/1 raster row header and row forwarding, including the original deferred empty-row handling in mode 1. The standalone ABI is project-authored; a full equivalent `_ProcessPage` including 2/3, global state and all branches is **not yet available**. GCC/Clang verification is part of the sources-only workflow and must be checked against the completed run for the associated commit.

## Verified source build: 2026-10-13

For commit `1345811cea4198e52554e8f3e0ac0738ceea2fd3`, workflow [Sources - Reconstruction Compile](https://github.com/an3k/leitz-icon-linux/actions/runs/38379977044) **completed successfully**: GCC and Clang built and exercised both isolated command and raster recovery test suites. This confirms **compilation and local test results only**, not executable parity with the original macOS Mach-O files or hardware behavior.

## 2026-10-13 — mode 2/3 row path recovery

Added `recovery/raster/classic_process_page_modes_23.[ch]`, based on the classic OEM `_ProcessPage` branches at `0x100007a74..0x100008882`. It recovers row-level original mode-2 bit runs, strictly shorter compressed selection, mode-3 edge treatment, dynamic offsets/widths, deferred blank rows, page-persistent width caches, and the distinct explicit reprint exception. The existing mode 0/1 source and `enhancements/` code were not changed. The full manufacturer function and original Mac binary parity remain unresolved. A GCC/Clang build workflow is triggered on this new commit; record actual outcome separately.

## Verified build — modes 2 and 3

For commit `86af3344029ed28d52fea915a453f065f5bdb3da`, the [Sources - Reconstruction Compile](https://github.com/an3k/leitz-icon-linux/actions/runs/38382200525) workflow completed successfully: the recovered classic command and raster fragments, including the newly added 2/3 row paths, built and passed their offline checks under **GCC and Clang**. The check compares known, disassembly-derived byte sequences; it is **not** an execution of the vendor Mach-O or a binary equivalence test. No unchanged manufacturer C files were modified.


## 2026-10-13 — measured OEM runtime parity (supersedes historical pending-test statements above)

The **actual original Leitz 5.1.465 x86_64 Mach-O programs**, downloaded
from Leitz's official installer and verified against the exact SHA-256 hashes
at the beginning of this file, were run on an Intel macOS-15 runner against
the separate recovered C executables. These comparisons do not involve a
physical printer.

- Original `raster2printer` vs recovered filter with **14 original RGB24
  `PrintQuality` choices × 2 resolution modes × 4 optimization modes =
  112 tested combinations**: all 112 emitted exactly the same stdout
  bytes in the tested two-page fixture. Original `Draft` uses the 1-bit
  path rather than RGB24.
- Two-page monochrome printing: for optimized modes 2 and 3 exact stdout
  parity; for unoptimized modes 0 and 1, the OEM sends **four additional
  bytes beyond every raw row's nominal stride**. The recovered source now
  reproduces the read size. For these modes the last four bytes of each page
  are undefined OEM memory, so comparison of **all defined bytes** is
  reported separately and must never be called universal exact parity.
- Four additional combinations with 33 and 127 raster rows, modes 2 and 3,
  `LabelCutter=OFF`: exact emitted bytes, including original end-of-job
  cutter behavior. There is no imposed fixed label length.
- Original `command2leitz` vs CUPS entry: seven cases covering recognized
  status/firmware queries, empty or failed backchannel response and an
  unrecognized command. A real macOS dynamic interposer supplies synthetic
  read responses to the original; the same responses are supplied to
  recovered code. Exit codes, raw stdout and `STATE:/ATTR:` lines match
  across every exercised case.
- `_OpenDebugFile` and `_SendDebugFileToPrinter` reconstructed from
  native original disassembly, including original mode bits 0755/0644
  and `fopen("wb")` / `fopen("r")`. The recovered native
  `PrintThroughPRN=ON` output path was then compared against the actual
  original: both produced an identical 3,620-byte intermediate PRN file
  and an identical 3,620-byte stdout stream for the verified test case.
- GCC, Clang, Intel macOS test builds and native macOS SDK CUPS adapter
  builds passed. Executions, not merely compilation, ground the comparisons.

Verified logs:
- [Command filter reference run](https://github.com/an3k/leitz-icon-linux/actions/runs/38431824113)
- [First 32 RGB24 cases](https://github.com/an3k/leitz-icon-linux/actions/runs/38432253626)
- [Remaining 80 RGB24 cases and dynamic lengths](https://github.com/an3k/leitz-icon-linux/actions/runs/38432927269)
- [Original PRN file transfer](https://github.com/an3k/leitz-icon-linux/actions/runs/38434844921)

**Not yet complete or proven equivalent for all input/control paths:**
the native `_ResolveDeviceURI` and `_CheckPrinterStatus` branches,
real IPP socket setup, original global logging and diagnostic helpers,
exception/termination behavior under all causes, full original function
ABI/layout and real-device communication. The test helper structures
are newly authored, and actual manufacturer C source was absent from
the installer. Neither the OEM's source code nor whole-executable
behavior has been reproduced in full. The safer alternative modules
under `enhancements/` remain separate.
