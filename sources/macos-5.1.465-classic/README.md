# macOS 5.1.465 — classic driver, PPD 1.2.190

Archive: `LeitzIconSoftware-Installer_5.1.465.dmg.zip`.

| Original Mach-O x86-64 executable | Size | SHA-256 | Evidence |
|---|---:|---|---|
| `raster2printer` | 84,252 bytes | `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` | Full disassembly obtained locally |
| `command2leitz` | 35,932 bytes | `6e43cae5e152d5e2d8e9e975d026375563afdffdadca08f835cb4c3a58d3f3ee` | Full disassembly obtained locally |

Mach-O debugging symbols identify original C compilation units including `raster2printer.c`, `command2leitz.c`, and shared `raster_c_functions.c`. These are **source filenames, not recovered full C sources**.

## Reconstruction status

- Both executable files extracted, identified as Mach-O x86-64.
- Instructions and available function symbols inspected; assembly listings retained in the working analysis archive.
- Functions for raster handling, compression, dithering, device queries, and status decoding identified.
- **No complete, compilable C reconstruction available.**
- No successful reconstructed build and no byte-level hardware equivalence established.

Linux 1.0.3 original C may guide identification of external APIs and shared functionality, but it is not evidence that the macOS code is identical. The macOS-specific branches must be established independently from its binaries.

Missing requirements for claiming completion: precise control/data flow reconstruction for every relevant function, global/static data recovery, Apple/CUPS API declarations, compilation of both recreated CUPS programs, and behavioral comparison against the original output on equivalent inputs.

Do not equate this classic model with the separately installed macOS V3 driver.
