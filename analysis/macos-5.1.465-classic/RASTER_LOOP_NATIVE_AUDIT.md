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
| `0x1000056a2..0x1000056c3` | If not reprinting, read CUPS header; otherwise branch to reprint handling | Existing connector covers only non-reprint branch |
| `0x100006128..0x1000061d6` | Reprint branch can call `_ProcessPage` with fourth argument `1` and a selected retained buffer | Not yet treated as fully reconstructed |
| `0x1000062b5..0x1000062e5` | Continue only if process status zero and original has-page bit is set | Existing connector has the normal outcome |

The manufacturer emits the non-reprint message **before**
`cupsRasterReadHeader2`; the message also appears on the iteration that
ultimately encounters EOF. The byte-exact literal is
`\n Not reprinted i_page = %i.\n` with the *current* zero-based page index.
The manufacturer's paired startup diagnostics are
`\nraster2printer.c:256:raster2printer(): started printing pages`
and the same string at source line `258`, without a trailing newline.

## Limits

This audit confirms concrete native addresses and format literals. It does
not prove that the complete native reprint path is reachable, unreachable,
or equivalent under all inputs; the compiler-local flag's **complete**
read/write inventory is being collected by the same script for independent
verification. The original C++ ABI and IPP/status transport are not
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
