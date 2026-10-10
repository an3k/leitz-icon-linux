# macOS 5.1.465: Leitz Icon status protocol from the original installed binaries

**Scope:** offline static inspection of the user-supplied original `LeitzIconSoftware-Installer_5.1.465.dmg`. The DMG is HFS+, containing a WiX-unrelated macOS `LeitzIconSoftware-Installer.mpkg` XAR package. `leitzprint.pkg` contains **four Mach-O x86_64 filters**, all inspected without execution:

- `LeitzPrint.bundle/Contents/MacOS/command2leitz`
- `LeitzPrint.bundle/Contents/MacOS/command2leitzV3`
- `LeitzPrint.bundle/Contents/MacOS/raster2printer`
- `LeitzPrint.bundle/Contents/MacOS/raster2printerV3`

The separately extracted `leitziconppd.pkg/LeitzIcon.ppd.gz`, PPD `1.2.190`, names the **non-V3** filters as `cupsFilter` targets. Presence of V3 binaries does **not** establish that this queue uses V3 in service.

## Command responses — directly supported by disassembly

| Request | Source function and evidence | Response interpretation | Unproven |
|---|---|---|---|
| `1B 41` ESC A | `command2leitz:GetPrinterStatus`; `command2leitzV3:GetPrinterStatus`; `raster2printerV3:CheckPrinterStatus` | First byte of backchannel reply is a **binary status code**, and later data (if any) is not needed by these tested branches | Reply total size and hardware support across firmware |
| `1B 55` ESC U | `GetUnitedStatus` in both command filters | Passed to CUPS/logging as string data with prefix; already proven from actual USB capture as `SKU:...;INIT:...;USED:...;\x00` | Units of INIT/USED; other SKU combinations |
| `1B 56` ESC V | `GetFirmvareVersion` (source spelling) in both command filters | Response is forwarded with text formatting `%s` and `FIRMWARE_VERSION_` prefix, with software-appended NUL after the received bytes | Exact version grammar, terminator emitted by device, sample |
| `1B 50` ESC P | **V3 raster only**, `GetNumberOfPrintedLines` | **Binary integer, most significant byte first**: for response bytes `b[0]...b[n-1]`, driver combines `b[i] << (8*(n-i-1))` into a 32-bit value. Interpreted as already printed line count | Whether actual response is always exactly four bytes and whether number is usable on user's firmware |
| `1B 75` ESC u | `command2leitzV3:GetWiFiStatus` | V3 query uses CUPS backchannel; unrelated to current USB print-completion goal | Reply schema not decoded in this analysis |

The V3 raster's `GetNumberOfPrintedLines` calls `cupsBackChannelRead(buffer,1024,7.0)`; the `CheckPrinterStatus` routine calls `cupsBackChannelRead(buffer,64,7.0)`. These are **maximum read buffer sizes**, not claims that the printer emits 1024 or 64 bytes. No current hardware capture covers A/P/V replies.

### First byte of ESC A reply: legacy macOS command filters

Both `command2leitz` and `command2leitzV3` use the simpler `GetStatus` mapping after a single `cupsBackChannelRead(buffer,1024,timeout)`:

- `0x03` → ready
- `0x20` → out-of-paper / media-empty
- `0x30` → paper jam
- `0x40` → general error

For ESC U/V the same `GetStatus` function uses the received bytes as a string for marker/log output; it is **not** a validated decoder for arbitrary binary replies. A negative or zero byte count is treated as no data.

### First byte of ESC A reply: `raster2printerV3` (extended decoding)

The V3 raster implementation uses this **ordered decision sequence**:

1. `status == 0x03` → ready.
2. `status == 0x04` → door open.
3. `(status & 0x02) == 0` → busy (active-low bit in this branch).
4. `(status & 0xfc) == 0xc0` → paper jam.
5. `(status & 0xfc) == 0xa0` → no/empty cartridge.
6. `(status & 0xfc) == 0x90` → printhead overheat.
7. `(status & 0xfc) == 0x88` → cutter jam.
8. Otherwise, it describes error bits using masks `0x02`, `0x04`, `0x08`, `0x10`, `0x20`, `0x40` and reports a general error.

**Do not** conflate the legacy `0x20` status with the V3 `0xa0` error encoding as interchangeable or active on this user's printer. These describe manufacturer-side *host decoder code paths* only; no specific byte observed from the actual hardware.

## Recovery and incomplete print jobs

The macOS V3 raster filter `CheckPrinterStatus` retries certain error states after sleeps (busy 1 s; various errors 5 s); read errors are bounded by a retry counter. It requests ESC P, stores returned printed-line count, and compares it with an expected line-count field before selecting whether the last label should be resumed. `ProcessPage` uses that count to derive `IndexOfStartLine`; one branch uses `printedLines` directly and another uses integer `(printedLines / 2)+1` depending on raster-mode path.

This **proves real manufacturer resume logic in a compiled V3 filter**, but does not establish that the ordinary PPD selects it or that ESC P is supported/monotonic across firmware, power loss, die-cut materials and cutting. It also does not prove independent confirmation of physical cutter completion.

The Windows 2018 language monitor independently maps `leitz_oem_status` to ESC A and `leitz_oem_version` to ESC V. Its 1024-byte response reader stops at requested length or NUL. The Windows `JOB_CONTROL_LAST_PAGE_EJECTED` notification is **spooler accounting** and not evidence of a device-side end-of-job confirmation.

## Implications for the Linux development driver

- Leave `main`/TARS untouched. Preserve the Windows-like verified print encoding.
- For any future experimental A/P/V probing, use **binary-safe bounded byte reads in a single USB-owning process**. Do **not** reuse Bash `read -d ''` for binary ESC A/P status; a leading zero is a meaningful possible byte.
- For ESC A decode, prefer a strict decoder mode matching the exact intended filter generation; unknown status bytes must remain `UNKNOWN`, not `READY`.
- For ESC P, accept a line count only after validating actual byte length, monotonicity and page/raster geometry; the upstream V3 code does not itself provide a strict 4-byte-length contract. A timeout or malformed reply must not trigger blind reprint.
- Keep backend `flock` for coordinating our processes; note it is advisory and stops guarding only once the transfer completes, **not** when the printer finishes moving/cutting.
- Our current `leitzusb` sends validated data in one bulk stream and cannot interleave progress queries safely. A status-aware backend will require an explicit **single-owner state machine** with controlled send/query phases, separate error states, and no unsupported automatic retries.
- A controlled, one-time real-device capture of ESC A, ESC P and ESC V at idle, plus ESC P during a predetermined safe print, is the remaining prerequisite for claiming device-level compatibility. No such USB query was sent during this analysis.

Binary inspection offsets for reproducibility (Mach-O x86_64 VM addresses): `raster2printerV3:CheckPrinterStatus` 0x100008d30; `raster2printerV3:GetNumberOfPrintedLines` 0x100009820; `raster2printerV3:ProcessPage` 0x100006e80; `command2leitz:GetStatus` 0x100001440; `command2leitzV3:GetStatus` 0x100005300.
