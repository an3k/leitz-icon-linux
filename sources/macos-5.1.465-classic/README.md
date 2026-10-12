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
