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
