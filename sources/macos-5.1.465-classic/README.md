# Original macOS classic driver — installer 5.1.465, PPD 1.2.190

| Component | Original size | SHA-256 | Branch contents |
|---|---:|---|---|
| `LeitzIcon.ppd` | 151,524 bytes | See git blob `14628cebde52eba5cc731b4679eda1471f50e312` | Original PPD, byte-identical |
| `raster2printer` | 84,252 bytes | `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` | Symbol inventory; full disassembly held in separate analysis bundle |
| `command2leitz` | 35,932 bytes | `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee` | Symbol inventory; full disassembly held in separate analysis bundle |

The two original executable files are Mach-O x86-64. Original debug metadata identifies `raster2printer.c`, `command2leitz.c`, and the shared `raster_c_functions.c`; these filenames **do not establish that the full original C source is present**.

Completed: binary identification, disassembly, initial function/branch identification, acquisition of PPD, symbol inventories.

**Not completed:** full decompilation into C, correct recreation of globals/constants and macOS dependencies, successful compilation, functional comparison on controlled equivalent inputs, hardware qualification.

Linux original C is useful as a reference for APIs and terminology only; no identical implementation is assumed. V3 is a distinct model-specific driver and is intentionally excluded from this stage.

No fabricated C code has been created merely to satisfy a file count.

## Disassembly-derived, compilable C fragments

The `reconstructed/` directory now contains real C source files for small portions of both classic macOS binaries. Built locally using GCC C11 with `-Wall -Wextra -Werror`; `make check` succeeded. No macOS runtime or USB hardware test was performed.

- `mac_classic_command_core.[ch]`: query letters for GetPrinterStatus / GetUnitedStatus / GetFirmvareVersion; ESC A response equality checks for 0x03, 0x20, 0x30, 0x40; ESC Z end marker.
- `mac_classic_raster_boundaries.[ch]`: StartDocument 120 padding ESC bytes, ESC @, optional ESC i; EndPage ESC G plus optional ESC E; EndDocument optional ESC E plus ESC @ ESC Z.
- `smoke.c` and `Makefile`: compile these isolated code fragments and check reconstructed byte sequences without CUPS or USB access.

Relevant disassembly addresses (Mach-O x86-64 virtual addresses):

| Original executable | Function | Address |
|---|---|---|
| command2leitz | GetPrinterStatus | 0x1000011d0 |
| command2leitz | GetUnitedStatus | 0x1000012a0 |
| command2leitz | GetFirmvareVersion | 0x100001370 |
| command2leitz | GetStatus | 0x100001440 (only fragments covered) |
| command2leitz | EndOfJob | 0x100001a30 |
| raster2printer | StartDocument | 0x100006890 |
| raster2printer | EndPage | 0x1000089b0 |
| raster2printer | EndDocument | 0x100008ac0 |

**Not reconstructed:** both complete `main` functions, CUPS option parsing, raster input, complete half-toning, page geometry, StartPage, ProcessPage, ReadPage, USB/socket output, macOS notifications, complete status backchannel, and all shared functions. The fragments are **not** a replacement for the originals. A successful fragment build is not a successful complete driver build.

## Reconstruction update: raster geometry and original half-toning

Reconstructed source in `reconstructed/` now additionally contains:

- `mac_classic_dither.c` / `.h`: independent disassembly-based reconstruction of `_DitherPage` (`0x100006dd0`). All **13 distinct manufacturer integer matrices** have been extracted directly from the classic Mach-O data segment, with source addresses and SHA-256 of each matrix block preserved in comments. The switch table maps 14 modes to these tables, dimensions, and floating-point scaling factors. Mode 7 falls through without an initialized table pointer in the observed binary and is rejected by the reconstructed safe API rather than dereferenced.
- `mac_classic_raster_boundaries.c` / `.h`: `_StartPage` (`0x100007380`) now emits the **original 11-byte page setup** `ESC L <length-hi> <length-lo> ESC w <width-hi> <width-lo> ESC B 0`. Its 300/600 dpi-dependent length arithmetic comes directly from the binary.
- `smoke.c` tests those page-setup bytes, dither mode dimensions/scales, representative black/white raster output and malformed input rejection.

For reproducibility on Linux with GCC:

```sh
cd sources/macos-5.1.465-classic/reconstructed
make clean check all
```

This successfully compiled and ran locally under `-std=c11 -Wall -Wextra -Werror`.

**Limitations:** This is still a **partial C reconstruction**. The full raster loop (`_ProcessPage`), document option handling, original CUPS API integration, complete `command2leitz` response/error control flow and executable entry points are **not** fully reconstructed. Behavioral equivalence to a running Mach-O filter has not been demonstrated. Matrix bytes are verified independently of behavior.

### Original `PrintQuality` selection

The `_raster2printer` case-insensitive choice comparisons at Mach-O virtual addresses `0x100004dbd..0x100005098` have now been reconstructed in `mac_classic_quality_from_ppd()`. This yields all **15 original PPD choices**, mapped to the exact integer selectors (0..14) used by `_DitherPage`. Choice `Draft` maps to selector 7 and sets single-bit raster input in the original PPD; it is intentionally not processed by the RGB24 dither routine. The 14 other selections map to 13 extracted manufacturer matrix arrays.

The selection mapping and its case-insensitive behavior are included in the successful local `make check` run. This documents and reconstructs **the quality selector**, not the remainder of the original printing application.
