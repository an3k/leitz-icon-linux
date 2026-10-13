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