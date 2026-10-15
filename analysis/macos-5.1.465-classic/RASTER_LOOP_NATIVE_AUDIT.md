# Original macOS Classic raster-loop transition audit

## Source and scope

The original `raster2printer` Mach-O from the Leitz 5.1.465 installer,
SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`,
was disassembled **directly** by `otool -tvV` in
[GitHub Actions 38546191969](https://github.com/an3k/leitz-icon-linux/actions/runs/38546191969)
after verifying the vendor executable hash. The bounded native
instruction audit is in `verification/macos-5.1.465-classic/audit_raster_transitions.py`.
This is manufacturer's machine code, not inferred from the separate Linux OEM source
or from the synthetic connected C helper.

## Previously missing, assembly-proven control-flow steps

| Original instruction | Original meaning | Reconstruction |
|---|---|---|
| `0x10000548e` | Call `_StartDocument` | Already recovered |
| `0x10000550e..0x100005523` | `fprintf` at source line `0x100` (256), message `started printing pages` | Add original stderr literal immediately after successful startup |
| `0x1000055ae..0x1000055c3` | `fprintf` at source line `0x102` (258), same message | Add second original stderr literal |
| `0x1000055ce` | Set original zero-based printed-page index to zero | Already recovered |
| `0x100005603` | Initialize native reprint bit to zero | Already initially modeled as non-reprint |
| `0x10000562d..0x10000563c` | Check `terminate_flag` before entering page-loop diagnostics | Retain existing termination check |
| `0x100005641..0x1000056a2` | Select original `Reprinted` or `Not reprinted` loop message | Add **only** the normal non-reprint message to the currently non-reprint connector |
| `0x1000056a2..0x1000056c3` | If not reprinting, read CUPS header; otherwise branch to reprint handling | Reachable non-reprint branch retained; nonactivated reprint handler now separately reconstructed and tested, without changing the main connector |
| `0x100006128..0x1000061d6` | Reprint branch selects retained mono/dither buffer, clears descriptor +0x24 and calls `_ProcessPage` with fourth argument `1` followed by `_EndPage` | Source-level branch recovered in `classic_original_raster_reprint_page`; standalone tests cover RGB24/mono calls; still unreachable in original defined normal execution |
| `0x1000062b5..0x1000062e5` | Continue only if process status zero and original has-page bit is set | Existing connector has the normal outcome |

The manufacturer emits the non-reprint message **before**
`cupsRasterReadHeader2`; the message also appears on the iteration that
ultimately encounters EOF. The byte-exact literal is
`\n Not reprinted i_page = %i.\n` with the *current* zero-based page index.
The manufacturer's paired startup diagnostics are
`\nraster2printer.c:256:raster2printer(): started printing pages`
and the same string at source line `258`, without a trailing newline.

## Limits

The initial instruction sampling confirms native addresses and literals;
the **later complete reference inventory below** establishes that this
function never directly sets its own reprint flag to one. It does not
prove behavior after undefined OEM memory corruption or certify an
actual manufacturer executable with the dormant branch activated. The original C++ ABI and IPP/status transport are not
reconstructed by these diagnostic corrections. Any added code belongs
only to the newly authored connecting C function, not to the manufacturer
original files, existing isolated helper functions, or the active driver.

## Complete native reprint-flag reference inventory

The second SHA-256-verified manufacturer run,
[38546921933](https://github.com/an3k/leitz-icon-linux/actions/runs/38546921933),
collected **every direct reference** to the original frame-local
`-0x7c05(%rbp)` in `_raster2printer` from `0x1000045a0`
to `0x1000062eb`, not only the selected blocks. The result:

| Address | Manufacturer instruction | Meaning |
|---|---|---|
| `0x100005603` | `movb $0x0, -0x7c05(%rbp)` | Single direct assignment: zero |
| `0x100005641` | `movb -0x7c05(%rbp), %al` | Read reprint flag for diagnostic |
| `0x1000056a2` | `movb -0x7c05(%rbp), %al` | Read for CUPS-header versus reprint branch |
| `0x100005f91` | `testb $0x1, -0x7c05(%rbp)` | Read for output selection |
| `0x1000060d2` | `movb -0x7c05(%rbp), %cl` | Read for status diagnostic |
| `0x100006128` | `testb $0x1, -0x7c05(%rbp)` | Read before optional repeated `_ProcessPage` |
| `0x1000061e0` | `testb $0x1, -0x7c05(%rbp)` | Read after optional repeated `_ProcessPage` |

**Finding:** In the complete inspected native function there is no
direct store of a nonzero value to this flag and no direct instruction
taking the address of that flag. Under defined execution of this
routine, the reprint-only branch therefore has no established activation
path. It remains present as *unreachable compiled manufacturer code*,
rather than an unimplemented ordinarily active feature. The original
contains unchecked buffer operations, so undefined memory corruption
cannot be ruled out by this static inspection alone.

Other recovered state slots have real transitions: the has-page flag
`-0x7c06(%rbp)` is assigned zero, one, then zero again at
`0x100005c1e`, `0x100005c2a`, and `0x100005cf1`,
respectively; the first-read flag `-0x7c19(%rbp)` is set to one
at `0x100005626` and cleared at `0x100005f34`.
The actual page-index increment is `0x100006227..0x10000622a`,
and the process-result/has-page continue gate is
`0x1000062ba..0x1000062e5`. The existing connected C path implements
the normal zero-reprint state, but it is **not** a complete recovery of
the manufacturer's native ABI or of its network/error behavior.

The audit now also lists every **direct native function call** in
this active function to identify genuine remaining integrations,
rather than inventing reprint behavior which the manufacturer does not
activate normally.

## Original function call inventory

[GitHub Actions run 38547714269](https://github.com/an3k/leitz-icon-linux/actions/runs/38547714269)
verified the manufacturer Mach-O SHA-256 and collected **153 direct
`callq` instructions** from `_raster2printer`,
`0x1000045a0..0x10000685a`. Every such instruction was
inspected, not just known print-command symbols.

| Original instruction | Direct target |
|---|---|
| `0x10000548e` | `_StartDocument` |
| `0x1000056d1` | `cupsRasterReadHeader2` |
| `0x100005bfc` | `cupsRasterReadPixels` |
| `0x100005fb9` | `_DitherPage` |
| `0x100005fc5` | `_StartPage` |
| `0x100006011` | `_ProcessPage` (ordinary branch) |
| `0x10000605e` | `_EndPage` (ordinary branch) |
| `0x100006189` | `_ProcessPage` (unactivated reprint branch) |
| `0x1000061d6` | `_EndPage` (unactivated reprint branch) |
| `0x10000632d` | `_EndDocument` |
| `0x10000662b`, `0x1000066e4` | `_WriteData` (epilogue) |

**No direct call** to `_CheckPrinterStatus` or `_ResolveDeviceURI`
occurs in this original function. Thus, inserting an extra status
query into the connected page loop would be **unsupported** by this
manufacturer code. Either function may still be used indirectly by
`_StartDocument`, `_StartPage`, `_ProcessPage` or other functions;
this audit does **not** determine those callee bodies. The existing
separate status/URI reconstructions are therefore preserved, not
arbitrarily spliced into the page loop.

The reprint-only pair of direct calls is present in the original
binary but conditioned on the compiler-local byte that has only the
documented direct zero assignment. Its observed native code is
not evidence that normal execution activates reprinting.

## Isolated original reprint caller reconstruction (2026-10-15)

The complete original flag audit was **already obtained** in
[38546921933](https://github.com/an3k/leitz-icon-linux/actions/runs/38546921933)
before this change. Two later native audit runs
([38735364670](https://github.com/an3k/leitz-icon-linux/actions/runs/38735364670)
and [38735555142](https://github.com/an3k/leitz-icon-linux/actions/runs/38735555142))
reconfirmed selected call instructions but did not establish a new flag
transition. Redundant additions to the helper-audit workflow were
subsequently reverted; the original established audit script and record
remain intact.

[Commit 9612314728e93671f649b35eff3db2f38d347c53](https://github.com/an3k/leitz-icon-linux/commit/9612314728e93671f649b35eff3db2f38d347c53)
recovers the previously missing **caller's dormant branch**
`0x100006135..0x1000061d6` as
`classic_original_raster_reprint_page` in the existing
`original_raster_active_page_output.c`, without reimplementing the
already recovered ProcessPage encoders. The branch zeros descriptor
offset +0x24; picks the **already retained** dither buffer for
cupsColorSpace==1, otherwise the original page buffer; calls
ProcessPage(descriptor,pixels,page_index,1); and invokes EndPage with a
full by-value descriptor copy. It does **not** call StartPage, dither
again, increment page index or free page memory.

The existing normal CUPS connector was **not** changed to activate this
inaccessible original branch. The isolated callback tests exercise both
pixel selection paths, retained buffer ownership, descriptor clearing,
the fourth ProcessPage argument and EndPage sequencing. Thus the tested
source control flow now includes both OEM call sites, without inventing
a missing transition or claiming executable-vendor parity for this
normally unreachable branch.

[CI 38736163664](https://github.com/an3k/leitz-icon-linux/actions/runs/38736163664)
passed all three Linux/native-macOS/vendor-comparison jobs. Existing
manufacturer comparisons remained 27/27 command cases and 90 connected
raster/PRN/IPP parity-success records, no reported PARITY=NO.

## 2026-10-15 — OEM raster-job epilogue repeatedly reads the live socket

**Newly checked native evidence:** The live `sources` HEAD before any work was `190ab753f7325a8c207b043cf94868d176c17bd7`. [Bounded OEM audit 38783910257](https://github.com/an3k/leitz-icon-linux/actions/runs/38783910257) verified the Leitz macOS 5.1.465 `raster2printer` executable SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13` before disassembling the original exit branch `0x100006440..0x100006530`. The original descriptor starts at `-0x10b0(%rbp)`. Hence the `cmpq $0x0,-0x20(%rbp)` at `0x1000064d6` tests a **64-bit slot at descriptor +0x1090** (not a read of the URI port +0x1098). Its original semantic member name/meaning remains undetermined. After the original `strstr(scheme, "ipp")` and 64-bit nonzero checks, `0x1000064e1` compares **the actual original file-private `__ZL4sock`** with -1; `0x1000064ee` reloads that global immediately before `close(fd)`. This is not an entry-time snapshot of a socket number.

A second [bounded OEM audit 38784545750](https://github.com/an3k/leitz-icon-linux/actions/runs/38784545750) inspected original `0x100006510..0x100006740`. The manufacturer reloads the same `__ZL4sock` into `%edx` at `0x1000065e1` immediately before the terminated-job six-byte reset's `_WriteData` at `0x10000662b`, and again at `0x10000669a` immediately before the conditional trailing one-byte `ESC` `_WriteData` at `0x1000066e4`. Both original calls copy the descriptor **by value**, size 0x10a8, and ignore the `_WriteData` result. The existing six-byte abort packet, counter increments, signed modulo 64, call order and data bytes were already correct and were not rewritten.

**Fixed integration gap:** [Code commit ce884ef365136602fd12506c845fe669963064b3](https://github.com/an3k/leitz-icon-linux/commit/ce884ef365136602fd12506c845fe669963064b3) adds an optional source-reconstruction `current_socket_fd` getter to the private `classic_original_raster_finish_ops` adapter. The joined macOS CUPS candidate binds this getter to its **already-existing** `status_socket_now`, which reads the single shared `classic_original_socket` / original Apple assembler label `__ZL4sock`. The epilogue invokes the getter separately at the close gate, close call, reset `WriteData`, and trailing `WriteData`. Existing isolated consumers without the getter continue using their prior explicit fd snapshot. No transport implementation, socket creation, status query or normal printer command was added.

**Tests and limitations:** A new isolated epilogue regression changes the synthetic live socket from 11 to 19 as a side effect of `close`, while the initial `socket_fd` snapshot is -1. It requires a close of 11 and BOTH output calls to use 19, proving fresh value retrieval across calls and the same 7 original reset/trailing bytes. [Test-adapter commit f1ddc2c490089e55ef7e052ca2ee4e2b1266340e](https://github.com/an3k/leitz-icon-linux/commit/f1ddc2c490089e55ef7e052ca2ee4e2b1266340e) fixes one unrelated old fixture's C positional initializer for this optional callback, without modifying production code. The first intermediate Linux build reported this compile-only missing initializer; final [CI 38785015592](https://github.com/an3k/leitz-icon-linux/actions/runs/38785015592) passed all 3 jobs (Linux GCC/Clang, native macOS Apple CUPS, authenticated OEM command/raster comparisons). The existing manufacturer cases report **27/27 command** success records and **90 connected OEM raster/PRN/IPP** parity successes, zero `PARITY=NO`. The live-socket change itself was validated from OEM disassembly plus the new isolated test, **not** by forcing an asynchronous socket mutation in the manufacturer executable or using a real device.

The descriptor 0x1090 slot is kept **opaque** (an 8-byte region) because only its width and zero/nonzero test, not a defensible original C++ declaration, have been established. The original `_CheckPrinterStatus` remains dormant in ordinary CUPS page printing. The native printer-status/network exception handling and physical device behavior are still incomplete.
