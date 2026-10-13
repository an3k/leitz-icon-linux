# Manufacturer reference sources

The manufacturer material in this directory must be distinguished from project-developed code.

| Manufacturer version | Authentic manufacturer C | Other material | Verified status |
|---|---|---|---|
| Linux 1.0.3 | `raster2printer.c`, `raster_c_functions.c` and their two headers | Original PPD and installer; separately created build instructions | All four C/header files byte-identical to the supplied original archive |
| macOS 5.1.465 classic | **Not supplied in the installer** | Original PPD, disassembly observations and recovered symbols | Neither `raster2printer` nor `command2leitz` has a complete validated C reconstruction |
| macOS 5.1.465 V3 | Not yet established | To be investigated separately | Not completed |
| Windows 5.0.3879.0 | Not yet established | To be investigated separately | Not completed |

## Separation of unverified or modified implementations

Previously developed, buildable macOS classic C reconstruction fragments have been preserved **without modification** under
[`enhancements/macos-5.1.465-classic/reconstructed/`](../enhancements/macos-5.1.465-classic/reconstructed/).
These are incomplete experimental implementations, **not** byte-for-byte manufacturer C, and their functional equivalence to either manufacturer binary has not been demonstrated. Their known intentional deviations must not be silently introduced into a fidelity-focused reconstruction.

The root-level `src/` and `cups/` directories are the project's own driver and integration, not additional manufacturer sources.

Acceptance requirement for future faithful reconstructions: recover the full execution path and data; produce a reproducible C build; compare supported behaviors and output against the **corresponding original manufacturer executable**; separately document any remaining uncertainty. Recompilation or project-only smoke tests do not prove fidelity.
