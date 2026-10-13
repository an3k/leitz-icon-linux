# Reconstructed original macOS classic `_test_raster`

The Leitz 5.1.465 original Mach-O executables contain essentially identical dormant `_test_raster` functions:

- raster2printer at 0x100003c60–0x100003e40
- command2leitz at 0x100004290–0x100004470

Both original binaries were extracted from the user-supplied manufacturer archive and SHA-256-verified before this reconstruction. This file was reconstructed from **both original disassemblies**, not copied from the project-developed implementation.

`original_test_raster.c` directly calls the original CUPS functions. It opens descriptor 0 in read mode, increments its page counter after every valid header, reports the original absolute source file's basename, line 358, current page number and `NumCopies` to stderr, allocates exactly `cupsBytesPerLine` bytes, and reads `cupsHeight` individual rows. It stops a page after **zero** returned pixels, accepts positive short reads, frees the allocated buffer if non-null, then continues reading page headers. On the first zero header it closes the CUPS raster handle.

The original has **no malloc-failure check**, and does not reject partial positive row reads. These omissions are retained here as source-reconstruction requirements. It MUST NOT be treated as a safer production utility.

The mock CUPS header under `fixture/` is test scaffolding only; the native macOS build uses real CUPS SDK headers. The isolated GCC and Clang tests check row counts, short-read acceptance, EOF and the exact diagnostic text. They are not evidence of full executable equality. `_test_raster` is **not called by the ordinary vendor printing path** and is not being introduced into that path.

This one recovered function does not finish either original executable. Native error/termination branches and several longer diagnostic/status helpers remain incomplete.
