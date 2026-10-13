# Reconstructed original macOS classic `_test_raster`

The Leitz 5.1.465 original Mach-O executables contain essentially identical dormant `_test_raster` functions:

- raster2printer at 0x100003c60–0x100003e40
- command2leitz at 0x100004290–0x100004470

Both original binaries were extracted from the user-supplied manufacturer archive and SHA-256-verified before this reconstruction. This file was reconstructed from **both original disassemblies**, not copied from the project-developed implementation.

`original_test_raster.c` directly calls the original CUPS functions. It opens descriptor 0 in read mode, increments its page counter after every valid header, reports the original absolute source file's basename, line 358, current page number and `NumCopies` to stderr, allocates exactly `cupsBytesPerLine` bytes, and reads `cupsHeight` individual rows. It stops a page after **zero** returned pixels, accepts positive short reads, frees the allocated buffer if non-null, then continues reading page headers. On the first zero header it closes the CUPS raster handle.

The original has **no malloc-failure check**, and does not reject partial positive row reads. These omissions are retained here as source-reconstruction requirements. It MUST NOT be treated as a safer production utility.

The mock CUPS header under `fixture/` is test scaffolding only; the native macOS build uses real CUPS SDK headers. The isolated GCC and Clang tests check row counts, short-read acceptance, EOF and the exact diagnostic text. They are not evidence of full executable equality. `_test_raster` is **not called by the ordinary vendor printing path** and is not being introduced into that path.

This one recovered function does not finish either original executable. Native error/termination branches and several longer diagnostic/status helpers remain incomplete.


## OEM _PrintPageDebugInfo in both classic executables

A second dormant helper was directly disassembled in both original Mach-O binaries: raster2printer 0x100003e50–0x100004419 and command2leitz 0x100004480–0x100004a49. Their seven diagnostic fprintf calls use identical original format strings and line numbers 385, 386, 387, 388, 389, 391, and 392. Both functions have been reconstructed in `original_page_debug.c` with the original filename basename and without additional null checks, path redaction or logging behavior changes.

The test-only CUPS fixture exposes only the fields needed for the isolated tests; on real macOS this file includes the actual CUPS raster SDK. The local GCC/Clang checks verify all seven exact output lines, and native SDK compilation is part of the sources CI workflow. No production raster/command entry point is changed to call this dormant helper. The executable as a whole remains incompletely reconstructed.

## Recovered OEM _output_cups_info (both classic binaries)

The two original macOS implementations are fully aligned at the function level with the existing **manufacturer's Linux 1.0.3 C source**. The active function bodies at raster2printer `0x100001ef0..0x100003c60` and command2leitz `0x100002520..0x100004290` share 1372 disassembly records, 108 calls and 140 literal references. The original vendor C body is preserved, with macOS's actual source diagnostic line numbers restored, in `original_output_cups_info.c`.

This is a **dormant diagnostic function**, not part of the normal printing path. Original behaviors including unchecked PPD pointers, fixed `sprintf` offsets, unusual concatenated formatting, and potential string-buffer overflows remain intentionally unchanged. Do not run it on untrusted PPD files. `ppd-fixture/` is only an offline mock of the CUPS data structures and is not shipped as a system header.

Offline compilation is tested with GCC and Clang. macOS native compilation against actual Apple CUPS headers is in `Sources - Reconstruction Compile`; this tests the source but does not execute the original private function directly or establish complete executable equivalence.
