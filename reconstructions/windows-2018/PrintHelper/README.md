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


## SendRecvData native COM call order and safe positive-result transfer

Native \`PrintHelper.dll\` i386 RVA 0x1720–0x191D was analyzed
offline from the supplied manufacturer's binary (SHA-256
\`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`).
No executable was launched. The original Export \`SendRecvData\`
accepts at least five 32-bit stack parameters at offsets
+0x08, +0x0c, +0x10, +0x14 and +0x18 (the last two are output
pointer destinations, assigned only in the nonempty result branch).
Source-level meanings of private COM methods are NOT presumed.

Confirmed CoCreateInstance pairs, using the native Win32 GUID
byte order (all GUIDs decoded from actual RVAs):
- Class at 0x443c: 2a614240-a4c5-4c33-bd87-1bc709331639
  with interface ID at 0x43ec:
  d580dc0e-de39-4649-baa8-bf0b85a03a97.
- Class at 0x441c: b9162a23-45f9-47cc-80f5-fe0fe9b9e1a2
  with interface ID at 0x43cc:
  8f348bd7-4b47-4755-8a9d-0f422df3dc89.

Original instructions and distinct vtable offsets:
1. 0x1762–0x179f: CoInitialize, CoInitializeEx, create
   first COM object, invoke vtable+0x0c with original first
   argument and literal 2. HRESULTs of object creation
   are not adequately guarded against invalid pointers.
2. 0x179f–0x17d7: create second COM object; invoke its
   vtable+0x0c using the exact UTF-16 text
   \`ReadWriteUserData\` at original RVA 0x41cc,
   **not** the formerly truncated \`ReadWriteUse\`.
   Invoke second object's vtable+0x10 with the literal 7
   and the original input buffer/length parameters.
3. 0x17d7–0x1822: invoke first object's vtable+0x14
   with the UTF-16 literal \`Get\` at original RVA 0x41f0.
   Invoke second object's vtable+0x14 and check BOTH its
   HRESULT and a separately returned zero DWORD at stack
   -0x18. Only when both are zero invoke the second
   object's vtable+0x18. The latter's result must equal
   zero to follow the positive-data branch.
4. 0x187c–0x18bb: only when the returned byte count at
   -0x2c is nonzero, allocate that number of bytes with
   HeapAlloc(GetProcessHeap,0,n), set the caller's
   output pointer and output length, and copy the COM
   buffer at -0x28 using memcpy. Original does not
   adequately verify HeapAlloc's result or the private
   COM-returned size; potential invalid reads/writes
   are NOT portable features to reproduce.
5. 0x18bb–0x18ec: free the COM-allocated source with
   CoTaskMemFree, release two COM objects and uninitialize
   COM. Native boolean return is true only if the COM
   returned a nonzero byte count; a zero-length or failed
   operation returns false. The Windows heap result has
   a distinct ownership and cannot be treated as borrowed
   memory.

\`sendrecv_result.[ch]\` is a NEW portable, bounded model of
**only** the positive-data result/length decision. It tests
all three required zero status/result fields, requires
nonzero reply length, transfers bytes only into an explicitly
bounded caller-owned destination, and leaves destination and
published length unchanged on failure. It never calls COM,
never allocates an unverified Windows heap block, and does
not claim the original DLL's ABI or wire protocol. Unit tests
exercise independent COM failure gates, positive binary data,
empty reply, invalid pointers/capacity and overlapping buffers.

No registration/IDL files identifying these private
interface methods were found in the uploaded ZIP. The
GUIDs and invocation slots are therefore **proven**,
whereas method names, internal call semantics and the
actual USB payload framing remain **unknown**. No device
command, firmware or Wi-Fi operation is performed.


## Windows PrintHelper SetPrinterWirelessNetwork: ANSI payload transfer

**Duplicate-work check:** sources HEAD
38ca610ae4b165c5763190e97f949a9d061dff72, prior passing
Windows CI 38911116602. No prior SetPrinterWirelessNetwork source
implementation was present. All existing LeitzRp, LeitzLm and
PrintHelper files/behaviors are preserved.

**Original manufacturer instructions:** \`PrintHelper.dll\` from
original uploaded ZIP, SHA-256
\`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`,
PE32 i386 ImageBase 0x10000000. Export \`SetPrinterWirelessNetwork\`
RVA 0x1150–0x120a uses arguments +0x08 (device-path string) and
+0x0c (configuration string). At RVA 0x1157 it calls helper 0x2140,
which wraps \`CreateFileW\` with \`GENERIC_READ|GENERIC_WRITE\`
for the supplied device path. At RVA 0x116b it creates an internal
UTF-16/BSTR-like holder for the configuration, then when needed
at RVA 0x1183 invokes helper 0x2ad0 to convert to bytes.
The helper uses \`WideCharToMultiByte(CP_ACP=0, flags=0)\` with
the UTF-16 input string length INCLUDING its trailing NUL;
the ANSI code page is selected by the Windows host and not a
fixed encoding supplied by the manufacturer.

At RVA 0x1194–0x11b1, the original computes \`strlen\` on the
converted byte string and passes that length (EXCLUDING NUL)
and the byte pointer directly to the common \`WriteFile\` wrapper
at 0x1e20. No ESC or proprietary prefix is assembled within
this export. The bytes are **opaque configuration data** supplied
by the caller; the content structure and firmware processing
remain unknown. A direct write to a chosen device path is NOT
the previously reconstructed \`SendRecvData\` private COM path.
The writer uses an overlapped event and may block on
\`ERROR_IO_PENDING\` via \`WaitForSingleObject(INFINITE)\`,
then calls \`GetOverlappedResult\`. There are native error
and null-pointer hazards; \`CloseHandle\` occurs after attempting
the write. No new USB command frames can be inferred from this.

**New portable bounded reconstruction:** the
\`wireless_payload.[ch]\` model accepts a bounded UTF-16 string,
a platform-provided ANSI conversion callback, and a synthetic
writer callback. It preserves the manufacturer's
\`WideCharToMultiByte\` then \`strlen\` boundary, including
truncation on converted embedded NUL and excluding the trailing
NUL from transmission. Unlike the OEM, missing terminators,
invalid conversion length and null pointers are rejected.
Temporary bytes are zeroized after conversion/delivery.
Non-ASCII handling is delegated to a codepage-aware callback,
not guessed to be UTF-8 or Latin-1.

**Proof boundary:** GCC and Clang strict compilation and
offline tests cover ASCII, a synthetic non-ASCII mapping,
early NUL, zero-byte payload, writer error, conversion error,
missing termination, oversize reports, buffer/pointer errors
and temporary-byte zeroization. These tests prove the new
portable wrapper's behavior against instruction-derived cases,
NOT full native Windows DLL execution parity, the unknown
configuration syntax, actual WLAN association or printer access.
No user credentials, network communication, firmware flashing
or actual device commands occur in tests.

**Separate firmware feature:** \`ReflashPrinter\` is the native
firmware-changing export at RVA 0x29a0, which must not be called
as part of discovery, print status or WLAN setup. Firmware
flashing remains expressly excluded from offline reconstruction
execution; it requires independent verification and physical
recovery provisions before integration.


## SendRecvData complete documented COM callback ordering

After the earlier isolated result-copy module, new standalone
`sendrecv_sequence.[ch]` joins the recorded native x86
RVA 0x1720..0x191d COM call ordering with that existing
`sendrecv_result.c` transfer. It preserves two distinct
CLSID/IID pairs, first-object slots 0x0c/0x14 and second-object slots
0x0c/0x10/0x14/0x18, literal flags 2/7 and exact UTF-16
`ReadWriteUserData` / `Get` arguments. It checks both the
second-object status HRESULT and its separate zero DWORD before calling
slot 0x18. Borrowed reply data is copied into caller-bounded storage,
then released via a separate callback before COM objects.

**Safety deviations:** Unexpected early callback failures stop safely
without dereferencing uninitialized COM pointers. The native
CoInitialize/CoInitializeEx, Windows BSTR/object ownership and precise
release order are abstracted into callbacks, not reproduced as Win32
functions or claimed byte-for-byte equivalent. In particular, this
is not a real private COM implementation or a printer transport.

`sendrecv_sequence_test.c` verifies original-identified GUIDs,
slots, flags, literal UTF-16 strings, successful nonempty and empty
replies, secondary status short circuit, all nine fail positions,
safe partial-resource cleanup, overlong replies, and validation.
GCC/Clang, AddressSanitizer and UndefinedBehaviorSanitizer passed
offline on test fixtures. No original DLL execution or hardware I/O.
