# Windows 2018 PrintHelper — DEVMODE patch reconstruction

Original source: user-supplied Windows ZIP, binary
\`LeitzIconLabelSetup/SourceDir/Leitz Icon Software/PrintHelper.dll\`
SHA-256: \`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`,
PE32 i386, image base \`0x10000000\`.

The original DLL exports \`SetPaper\` at RVA 0x1b30 and \`SetCustomPaper\`
at RVA 0x1930. Both open the Windows printer, load a DEVMODEW through
\`DocumentPropertiesW\`, update DEVMODEW and manufacturer-private bytes,
then write the changes through Windows \`DocumentPropertiesW\` and
\`SetPrinterW\` (level 9). These are **Windows printer configuration
operations**, not evidence of additional direct device wire commands.

Reconstructed C module \`devmode_patch.[ch]\` covers ONLY the defined
DEVMODEW byte modifications:
- offsets +0x44/+0x46 determine the full DEVMODE/private-data span;
- +0x48 gets \`dmFields=0x2402\`;
- +0x4e/+0x5a/+0x60 set paper size, print quality, Y resolution;
- OEM-private fields are located at the negative offsets -0x10,
  -0x0e, -0x0c and -0x0a from \`dmSize+dmDriverExtra\`;
- \`SetPaper\` checks the *existing* \`DM_PAPERSIZE\` bit before any
  changes, then sets private -0x10, fixed 1 at -0x0c, -0x0a;
- \`SetCustomPaper\` does not require that flag and sets private
  -0x10, -0x0e, -0x0a, leaving -0x0c unchanged.

The new API has explicit bounds validation and never calls Windows
spooler or hardware. It does not claim OEM x86 C++ ABI, all
exception paths, or Windows dynamic runtime parity. It is NOT
integrated into the native Linux CUPS driver, which already has
independent job-dependent custom-size handling.

Run \`sh test.sh\` for GCC/Clang fixed-vector checks. No OEM binary,
firmware, driver changes, or hardware commands are committed.

## GetPaperDefs and RetrieveStatus — original Windows spooler functions

The manufacturer PrintHelper.dll 2.17.4.112 i386 exports
GetPaperDefs (RVA 0x1210–0x1500) and RetrieveStatus
(RVA 0x15f0–0x1715). These are **Windows spooler/GDI functions**.

GetPaperDefs takes caller-specified paper IDs and a count. The native
x86 caller ID array uses 4-byte cells (RVA 0x13d3 reads the low WORD
at base + index*4); the new portable C interface accepts compact
normalized 16-bit IDs and does NOT claim the native input ABI. For each
paper it sets DEVMODEW dmPaperSize, invokes SetPrinterW level 9, creates
a new GDI device context and calls GetDeviceCaps in the exact order
112 (PHYSICALOFFSETX), 113 (PHYSICALOFFSETY), 110 (PHYSICALWIDTH),
111 (PHYSICALHEIGHT), 8 (HORZRES) and 10 (VERTRES), storing each
in its own integer array. This is per-paper **raster geometry**, not
paper cassette inventory or confirmed physical printable stock.
The original routine does not explicitly restore the previous paper
selection in its normal iteration. The new paper_geometry.[ch] is
an isolated, callback-driven **offline model**. It does not call
SetPrinterW or modify any printer settings. A failure leaves the
current incomplete record unchanged.

RetrieveStatus invokes Winspool GetPrinterDataW for the supplied
named printer property. This is a **spooler-stored value**, not an
immediate USB ESC A/U/V status exchange. The original reads up to
128 bytes into a stack buffer but then copies pcbNeeded bytes without
validating the returned length. The new printer_data.[ch] preserves
the documented retrieval relationship using a safe 128-byte limit,
rejecting Win32 errors and overlong buffers. No registry value names
or response meanings are invented; test keys are synthetic.

These new portable APIs do not claim binary compatibility with the
OEM DLL, Windows allocation ownership or direct Windows execution
parity. Their purpose is to preserve the proven GDI/status boundaries
for later Linux integration. The existing Linux driver is not changed.

Run sh test.sh for independent strict GCC and Clang regression tests.


## Queue work-offline attribute and direct ESC V version probe

The original PrintHelper.dll 2018 i386 exports \`IsPrinterOnline\`
(RVA 0x1510..0x15ef) and \`CanReadFromPrinter\` (0x1110..0x114a).
These methods do NOT perform the same test:

- \`IsPrinterOnline\`: opens a Windows printer and loads
  \`PRINTER_INFO_2\` via two \`GetPrinterW(level=2)\` calls, then
  tests DWORD at x86 offset +0x34 against 0x400.
  That is \`PRINTER_INFO_2.Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE\`,
  NOT the separate \`Status\` DWORD at +0x48. A FALSE value means
  opening/querying the queue failed or the queue's work-offline
  attribute is set, **not** confirmed hardware unavailability.
- \`CanReadFromPrinter\`: calls internal version query at 0x2270,
  sending exactly \`ESC V\` via Win32 file I/O, reading up to
  64 bytes, and checking for the case-sensitive substring \`BOOT\`.
  The BOOT flag is set to 1 only when found, never cleared by this
  routine. The exported wrapper returns TRUE when its internal
  routine returned 1 (successful send/read path), independent
  of the BOOT marker. The wrapper is not a general error/status
  decoder. The OEM uses unbounded \`strstr\` on a fixed 64-byte
  scratch; our test-only classifier uses bounded input.

\`status_probe.[ch]\` is a pure, portable C model of these two
separate decisions, NOT the native Win32 API, COM ABI or a USB
implementation. It does not issue \`ESC V\` or contact the printer.
\`status_probe_test.c\` covers both decisions and bounded BOOT parsing
under GCC/Clang. No production driver changes or hardware tests.
