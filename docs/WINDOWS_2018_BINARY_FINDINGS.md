# Windows 2018 installer: verified binary findings (static analysis)

Source: user-supplied `Leitz-Icon-Windows-Release.exe` extracted as `MSI.zip`, inside `LeitzIconLabelSetup.msi`. Analysis performed offline, without executing Windows programs or sending printer commands.

## Provenance and component boundaries

- INF: `Drivers/OEMPRINT.INF`; `DriverVer=03/15/2018,2.17.4.112`. The host may be Windows 11 24H2; the driver itself is dated 2018.
- Native driver files, both `amd64` and `i386`: `LeitzRp.dll` (raster plug-in), `LeitzLm.dll` (language monitor), `LeitzDevMode.dll`, `LeitzUI.dll`, `LeitzUSRes.dll`.
- `LeitzIcon.gpd` is readable manufacturer printer-definition data and uses `*MasterUnits: PAIR(300, 600)`, `CmdStartDoc` (120 ESC + `@`), `CmdStartPage` (`ESC B 00`), `CmdEndJob` (`ESC E ESC @ ESC Z`), media-specific `ESC L` and `ESC w`, and `ESC h/i` for resolution.
- **Corrected exact count** for this copy of `LeitzIcon.gpd`: **76 PaperSize options = 75 fixed + CUSTOMSIZE**. Prior notes describing 76 fixed Windows options are incorrect. The 86-profile development PPD contains 43 A names and 43 Leitz-prefixed B names, including 5 additional aliases over the previous branch version; a PPD count is not a count of distinct physical materials.
- `LeitzRp.dll` exposes COM exports `DllCanUnloadNow` and `DllGetClassObject`. Static UTF-16 text and symbols include `FilterGraphics`, `LeitzRp.cpp`, `shPrevEmptyLineCount`, `IsDieCut`, `ModeAuto`, `iLabelLength`, `LabelsCount`, and an **end-of-job 64-byte alignment message**. This establishes code paths/diagnostics, **not** the exact predicate or a safe padding requirement; **do not implement padding based only on this message**.
- `LeitzLm.dll` exports `InitializePrintMonitor2`. Disassembly of the `amd64` module's dispatch function at relative virtual address 0x1000 identifies:
  - `leitz_oem_status` → bytes `1B 41` (ESC A).
  - `leitz_oem_cartridge` → bytes `1B 55` (ESC U).
  - `leitz_oem_version` → bytes `1B 56` (ESC V).
  - Unknown request identifiers use a separate unmatched path.
  - Further status reads use a worker and a bounded buffer; **response framing/semantics are not yet independently established**.
- `PrintHelper.dll` is a **separate native user-mode helper** with exports `CanReadFromPrinter`, `GetPaperDefs`, `IsPrinterOnline`, `ReflashPrinter`, `RetrieveStatus`, `SendRecvData`, `SetCustomPaper`, `SetPaper`, `SetPrinterWirelessNetwork`.
- Managed `LeitzIconPrinterStatusService.dll` references `PrintHelper.dll` and method names `GetStatus`, `GetPrintStatus`, `GetVersion` and `RetrieveStatus`. This is not proof the language monitor alone implements these functions.
- The helper includes filename references `/leitz_icon_v3.bin` and `/eagle.flash.bin`, but neither firmware image was found in the extracted MSI zip. **No flashing or firmware modification is intended**.
- No original standalone C/C++/C# source files or PDBs were found in the MSI extraction. .NET assemblies contain IL/metadata suitable for further examination; PDB path strings embedded in binaries are **not** the PDB files.

## Implementation policy

Keep the currently physically tested Windows-like USB printing command path; the Linux 1.0.3 source and macOS legacy/V3 binaries are additional references, not automatic replacement implementations. Probe ESC A/V only with strict limits, kernel `usblp` exclusive open, and operator-confirmed mechanical idle; actual response framing remains to be captured. Completion must not be advertised as confirmed unless the device supplies verifiable evidence. Do not automatically append ESC alignment bytes or reconfigure production TARS.
