# Windows LeitzRp 2.17.4.112 — partial portable C reconstruction

**Scope:** The original x64 raster path at relative virtual addresses
0x26df–0x274b and 0x2907–0x29a7 of the manufacturer's `LeitzRp.dll`.
A standalone GCC/Clang test verifies output bytes and the 16-bit
previous-width state. The Windows COM ABI is **not** claimed.

Run `sh test.sh`. No Windows executable runs, no hardware is accessed.

The independent row_control.c module now reconstructs the OEM's
per-row ESC w / conditional ESC E / queued-blank ESC f / label-end ESC G
transition at x64 RVA 0x2060–0x237a. The original state offsets
+0x08/+0x0a/+0x0e/+0x10/+0x12/+0x18/+0x1c are documented
individually; its C struct is a new portable interface, not the native
C++ object. The row_index is managed by the original's outside caller.
An encoder callback is preserved in the standalone row controller; the RLE
codec itself is independently recovered and tested below.

The original x64 and x86 RLE body is now implemented in the independent
`rle_windows_2018.[ch]` module. It accepts already-trimmed byte rows,
retains 1..128-bit black/white runs, and returns USE_RAW unless output
is strictly shorter than input. The optional last-byte copy-and-mask
helper matches the manufacturer's conditional physical-width masking
without modifying caller-owned source memory. Test vectors are checked
against an independent bit-by-bit reference and the original
disassembly; this is **not direct execution parity against Windows**.

Not yet reconstructed: full automatic label-length control,
original COM object and Windows shell integration. The core RLE and
safe final-byte source clipping are separate new C test interfaces,
not copies of original OEM function signatures.

Remaining: caller-owned row index lifecycle, custom-length ModeAuto and
IsDieCut decisions, and original COM entry points. None of these
experimental fragments replaces the Linux CUPS driver. This code does not replace the active
Linux CUPS filter. See `analysis/windows-2018/RECONSTRUCTION_STATUS.md`.

The combined `rle_pipeline_test.c` exercises the existing prefix writer,
full RLE byte run computation and 0x16/0x17 payload selector together,
including byte-level output commands and their separate callback calls.
No production Linux code or COM entry point is changed.


The x64 OEM start-of-job media/length initializer RVA 0x16f7–0x1c4e
has now been reconstructed in label_setup.[ch]. It initializes the
already-existing row controller without reimplementing its output
functions, reading original paper-name flags ("Continuous" and
"CUSTOMSIZE"), ModeAuto, iLabelLength, dmOrientation and the GDI
raster-width/raster-height pair. It reproduces the conditional
landscape swap, diecut/custom-size flags, configured-versus-
automatic length and 16-bit native state truncation. No fixed length
cap is introduced. The original OEM +0x18 flag used by ESC w is
now positively identified as IsCustomSize. Its original C++ ABI
and all run-time Windows APIs are not duplicated or claimed.

Integration of label setup with previously recovered row control: `label_setup_row_test.c` checks that original-derived
`CUSTOMSIZE` enables per-row ESC w and terminates at raster height;
continuous manual-length sends ESC f/G at the provided iLabelLength;
continuous automatic-length terminates at raster height instead.
No encoder or USB backend changes are involved.


## Complete counted-row stream (offline adapter)

The standalone `raster_stream.[ch]` composes the previously
reconstructed media initialization, original-derived row state, prefix
width/margin commands, bit RLE encoder and raw/compressed selector into
one bounded C callback stream. It does not rewrite those source modules.
Its encoder uses caller-owned, separate scratch buffers and masks unused
final bits only in a copy. A per-row diagnostic captures an ignored
FilterGraphics result without changing the native-inspired row return.

`raster_stream_test.c` covers two consecutive synthetic labels,
raw versus RLE paths, blank-run feeding, medium switches, cache reset,
manual continuous length, conditional final-bit clipping, separate
WriteData call boundaries and failure/buffer behavior.

The vectors are derived from already documented OEM fragments; **neither
the Windows original nor a physical printer was executed**. This is not
the Windows COM/Unidrv caller, and the physical USB backend, native
exception semantics and OEM-vs-reconstruction parity remain open.
Run `sh test.sh` for GCC and Clang checks.


## Original LeitzRp.dll: OEM raster COM factory and Windows Unidrv host boundary

Preflight confirmed sources HEAD `f1b38b9f2b1d22aae0dcf73d556e16a249d37b86`,
successful previous Windows CI, and **no** existing LeitzRp
`GetInfo`, OEM driver interface binding, or COM class factory module.
Previous row/raster/RLE/length/status implementations were not altered.

The original user-supplied manufacturer ZIPs were opened and verified:
Linux 1.0.3 SHA-256
`54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17`;
Windows ZIP SHA-256
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`;
macOS Classic ZIP SHA-256
`6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210`.
For this specific function reconstruction the exact Windows
`Drivers/i386/LeitzRp.dll` and
`Drivers/amd64/LeitzRp.dll` were newly extracted and
disassembled, **never executed**:

- x86: SHA-256 `7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33`
- x64: SHA-256 `876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787`

### GetInfo and driver-interface acquisition

New standalone `oem_host.[ch]` covers the original
raster-plugin GetInfo (x86 RVA 0x1ed0, x64 RVA 0x1f20):
- mode 1: write requested length 4, return DWORD 0x4c504944
  (little-endian ASCII `DIPL`), S_OK.
- mode 3: write requested length 4, return DWORD 1, S_OK.
- mode 4: write requested length **8**, return E_FAIL **without**
  calling SetLastError. This differs from the separate LeitzUI
  implementation and is preserved verbatim.
- other modes/missing required-size pointer: E_FAIL,
  SetLastError(87), do not overwrite required size on unknown mode.
- missing/short output for modes 1/3: requested length 4,
  E_FAIL, SetLastError(122).
- The function writes no device/print/cutter command.

At x86 RVA 0x1f80 / x64 RVA 0x1ff0 the raster OEM object caches
a driver-provided COM interface at native +0x08 / +0x10.
Only when the slot is NULL does it invoke the incoming
IUnknown::QueryInterface for **exact GUID**
`d67ebbf1-78bf-11d1-9480-00a0c90640b8`, recovered
from x86 original .rdata RVA 0x5074 and x64 original .rdata RVA 0x5268.
Negative signed HRESULT clears the cache and maps to E_FAIL.
Nonnegative result (including S_FALSE) returns S_OK even for a NULL
result pointer; an already cached non-NULL pointer avoids a re-query.
The original raster object's destruction/reset calls Release on
its cached driver interface. The portable model uses explicit callbacks,
does not construct native Win32 COM objects, and rejects NULL
callbacks safely instead of relying on native unchecked pointers.

### Native plugin COM class factory

New `oem_factory.[ch]` covers the disassembled
raster-plugin exports and QueryInterface dispatch:

| OEM routine | x86 RVA | x64 RVA | Recovered operation |
| --- | --- | --- | --- |
| DllGetClassObject | 0x11a0 | 0x1190 | Compare one CLSID, allocate factory, QueryInterface and Release |
| Factory QueryInterface | 0x1220 | 0x1230 | Only IUnknown or IClassFactory |
| CreateInstance | 0x10d0 | 0x10a0 | Reject non-NULL outer object; create raster object; QueryInterface/Release |
| Raster QueryInterface | 0x1000 | 0x1000 | Only IUnknown or OEM raster-interface IID |
| LockServer | 0x1140 | 0x1140 | Atomic increment/decrement of one DLL counter |
| DllCanUnloadNow | 0x1180 | 0x1170 | S_OK iff two *other* original counters are both zero |

Relevant exact manufacturer GUIDs (Windows little-endian arrays in C):
- Raster-plugin CLSID:
  `6d6abf26-9f38-11d1-882a-00c04fb961ec`
  (x86 RVA 0x4174, x64 RVA 0x4280).
- Raster COM interface:
  `d67ebbf0-78bf-11d1-9480-00a0c90640b8`
  (x86 RVA 0x5084, x64 RVA 0x5278).
- Host COM interface:
  `d67ebbf1-78bf-11d1-9480-00a0c90640b8`
  (the different GUID in `oem_host`, **not** an alias).
- IUnknown and IClassFactory are the standard UUIDs, confirmed in
  both OEM binary vtable QueryInterface comparisons.

**Important OEM counter distinction:** native x64 LockServer modifies
RVA 0x76e4; x64 DllCanUnloadNow instead checks RVAs **0x76ec**
and **0x76e8**. Native x86 LockServer modifies 0x6430;
DllCanUnloadNow checks **0x6438** and **0x6434**.
These are distinguishable original storage locations, so the
portable implementation accepts them as separate supplied counters.
It does **not** presume that the lock counter automatically controls
the DLL-unload decision, nor invent the lifetime of these globals.

### Boundaries and validation

Both modules are portable C **decision models**, not Windows DLL
replacements. They preserve original verified IDs, HRESULT branches,
buffer sizes, caching and unload decisions. Allocation successes
and DLL global counts are explicit simulation inputs. Native C++
vtable pointers, COM object creation/ref-count ownership, Win32
function calling conventions, IPrintOemUni2 hooks and user-mode
GPD/Unidrv command timing are not claimed implemented.
No actual printer traffic or new paper length limit was introduced.

Regression programs `oem_host_test.c` and
`oem_factory_test.c` verify original byte-order GUIDs,
mode-specific status/errors, short-buffer nonmutation, interface
caching, negative/nonnegative HRESULT handling, aggregation,
class/interface rejection and all three distinct counter states.
GCC/Clang strict C11 and GCC AddressSanitizer/UndefinedBehaviorSanitizer
passed locally. The original LeitzRp `test.sh` is
extended to run both new cases alongside all existing raster tests.
CI on `sources` is the authoritative shared regression.


## Original LeitzRp 26-slot raster COM method vtable

Original Windows i386 LeitzRp.dll SHA-256
7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33;
amd64 SHA-256
876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787.
From original PE binary VTables (i386 RVA 0x509c; amd64 RVA 0x5298),
**both** have exactly 26 slots. This is a new part, separate from
previously reconstructed OEM factory and COM host binding.

Direct opcode checks: slot 5 returns 0, slot 9 returns 1,
slots 13, 14 and 16–25 (12 total) return E_NOTIMPL (0x80004001).
Slot 11 compares a name against exact literal FilterGraphics;
the original result is 0 on exact equality and 1 on mismatch.
No assumption of a COM capability boolean is made from this return.
The other slots are labelled native_method, NOT treated as
implemented or unsupported hardware features.

New portable C oem_method_map.[ch] and generated initializer preserve
the x86/x64 original RVA values and proven direct return decisions.
Original raw binary verification:
analysis/windows-2018/verify_original_rp_vtable.py optionally receives
--windows-zip, --linux-zip and --macos-zip. It independently reads the
Windows ZIP and original PE section tables, verifies 26 x86 and 26
x64 method addresses, native return opcodes and both original
FilterGraphics strings. It also verifies Linux 1.0.3 and macOS
5.1.465 original raster2printer SHA256s. All three archive checks
**passed in this session**. GitHub CI only checks committed JSON/C
consistency, since OEM ZIPs are intentionally not committed.
GCC, Clang, AddressSanitizer and UndefinedBehaviorSanitizer passed.

The VTable data is for future exact ABI integration and is NOT an
actual Windows COM object/vtable, nor a full native Windows driver.
All previous source fragments, production Linux driver and
printer hardware are unchanged.


## Reconstructed IUnknown object references (original LeitzRp.dll)

The new `oem_lifetime.[ch]` is an **isolated portable reference-count
model**, not an implementation of a native COM allocation or vtable.
It complements, but does not change, the already recovered
`oem_factory` and `oem_host` modules.

**Direct original observations** (both x86/i386 and x64/amd64 DLLs
from the uploaded Windows OEM ZIP):
- x86 `AddRef` at RVA `0x1070` applies
  `InterlockedIncrement` to the raster reference DWORD at object
  offset `+0x04`. x64 at RVA `0x1060` uses an atomic
  `lock xadd` at object offset `+0x08` and returns the new count.
- x86 raster `Release` RVA `0x1290` calls
  `InterlockedDecrement` on `+0x04`; x64 RVA `0x1290`
  uses `lock dec` on `+0x08`. Nonzero references return
  their remaining count without destroying the object.
- x86 `CreateInstance` RVA `0x10f4` initializes
  refcount `1`, increments tracked DLL-global RVA `0x643c`
  and clears cached host interface `+0x08`.
  x64 RVA `0x10dd` initializes refcount `1`, increments
  tracked DLL-global RVA `0x76f0`, and clears host `+0x10`.
- At the final raster release, native x86 destructor RVA
  `0x12d0` and x64 release RVA `0x129f` invoke
  the cached host's `IUnknown::Release` once if non-NULL,
  clear the host pointer, decrement the **tracked object global**
  and free the native raster object.
- Factory objects follow the same reference-count transitions
  but do not modify the raster-object global. Original native
  factory allocations are 8 bytes on x86, 16 bytes on x64.

**Critical distinction:** x86 `LockServer` uses global `0x6430`,
while `DllCanUnloadNow` reads `0x6438/0x6434`;
the *raster object tracker* is `0x643c`. x64 uses
`0x76e4`, `0x76ec/0x76e8`, and `0x76f0` respectively.
These addresses are individually observed in the original.
A conventional one-counter COM implementation would not be
source-equivalent here.

The new C code provides reference-count transitions, the
separate tracked global and one-time host-release callback with
explicit overflow/underflow checks. Unlike the manufacturer
implementation, it **does not free the caller's object memory**
or make native COM vtable calls; its behavior is deliberately
safe if called again after reaching zero. Actual multi-threaded
COM object ownership and release/destruction concurrency
are not claimed reproduced.

`oem_lifetime_test.c` checks factory/raster references,
AddRef/Release return values, one-time host cleanup, independent
counters, invalid buffers and overflow/underflow. Both GCC/Clang
(strict C11) and GCC ASan/UBSan passed locally. The existing
`LeitzRp/test.sh` now includes the new test, with no changes
to preexisting C implementations. OEM DLLs were disassembled
offline only; no Windows program or physical printer ran.

## Original Windows Raster-Methodenaufruf #15 – gestanzt versus endlos

**Zuvor offene Schnittstelle:** Die bestehenden Module
`raster_stream.[ch]` und `row_control.[ch]` modellieren die
Zeilenroutine des Windows-OEM bei x64 RVA `0x2060`, erwarten
aber bereits extern erhöhten `row_index`. Die Unit-Tests setzen
diesen Wert bisher manuell; das war **noch kein Originalbeleg für
den Endlosmaterial-Aufrufpfad**.

Die Hersteller-DLLs `LeitzRp.dll` aus dem Original-Windows-ZIP
(SHA-256 des ZIP:
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`)
belegen jetzt den unmittelbaren VTable-Einstieg der Rasterzeile:

| Herstellerpfad | x86 RVA | x64 RVA |
| --- | --- | --- |
| Original COM-Rastermethode, Slot 15 | `0x1FF0` | `0x2040` |
| 16-Bit `row_index++` | `0x1FFF` (Medium +0x0A) | `0x2044` (Medium +0x0E) |
| IsDieCut-Prüfung | Medium +0x10 | Medium +0x14 |
| IsDieCut != 0: gestanztes Material | `0x2020` | `0x2060` |
| IsDieCut == 0: Endlosmaterial | `0x2380` | `0x2380` |

**Neu:** `oem_row_dispatch.[ch]` ergänzt nur den im Original
belegten 16-Bit-Zeilenindex-Inkrement und die originale
`IsDieCut`-Auswahl. Der gestanzte Pfad ruft das **unveränderte**
`leitz_win2018_stream_counted_row` auf; damit gibt es keine
Duplikation des RLE-Encoders oder des ESC-G/ESC-E-Zeilencontrollers.

**Wichtig für Endlosmaterial:** Die native Methode bei `0x2380`
ist ein eigenständiger Rasterverarbeitungspfad. Dessen komplette
Logik wurde noch **nicht** nachgebaut. Deshalb verlangt der
portable Dispatcher für `IsDieCut == 0` einen eigens
bereitgestellten Callback; fehlt er, gibt er
`E_NOTIMPL` zurück und erzeugt **keine erfundenen Druckbytes**.
Die vorexistierenden Tests für manuelle Endloslänge beim
separaten `row_control` prüfen lediglich die bereits rekonstruierte
Teilroutine, nicht diese neue tatsächliche Windows-Aufrufverzweigung.

Offline-Tests:
- `oem_row_dispatch_test.c` isoliert die Aufrufsteuerung:
  Zählerinkrement vor der ausgewählten Routine, Rückgabewerte,
  16-Bit-Überlauf, ungültige Eingaben und nicht implementierter
  Endloszweig.
- `oem_row_dispatch_integration_test.c` bindet das **bestehende**
  `label_setup`, `raster_stream`, `row_control`, `RLE`
  und das Bytefiltermodul ein: Ein dreizeiliges `CUSTOMSIZE`
  erzeugt `ESC w`, `ESC f` und `ESC G`, ohne dass der
  Aufrufer den Zeilenindex manuell setzt. Endlosmaterial
  löst dagegen explizit den separaten Callback aus.
- Der kleine Dispatcher und sein isolierter Test wurden lokal
  mit GCC, Clang und ASan/UBSan ausgeführt. GitHub CI baut
  den zusätzlichen Integrationstest gegen die unveränderten
  realen Rekonstruktionsmodule.

Die Originalarchive `LeitzIconDriversForLinux1.0.3(4).zip`,
`LeitzIconLabelSetup(5).zip` und
`LeitzIconSoftware-Installer_5.1.465.dmg(2).zip`
wurden erneut aus ihren tatsächlichen ZIP-Bytes geprüft.
`raster2printer.c` (Linux), `LeitzIcon.gpd` (Windows)
und `raster2printer` (macOS Classic) stimmen jeweils in der
SHA-256 mit den zuvor gespeicherten Originalbelegen überein.

Es werden keine Windows-COM-Objekte, keine Hardwarefunktionen,
kein USB-Datenstrom und keine produktiven Linux-Branches verändert.


## Manufacturer's actual separate continuous-media raster path (RVA 0x2380)

**New functionality, not a repeat of previous row_control:** Original
Windows LeitzRp.dll (version 2.17.4.112) sends the IPrintOemUni raster
slot 15 into a distinct function at RVA **0x2380** when
`IsDieCut == 0`. The previously reconstructed `row_control.c`
models the other branch (die-cut). The three original manufacturer
archives were directly opened and their original Linux
`raster2printer.c` SHA256, Windows i386/amd64 `LeitzRp.dll`
SHA256 and macOS Classic `raster2printer` SHA256 were reconfirmed:
the Windows DLL SHA256 values are
`7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33`
(x86) and
`876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787`
(x64).

Both actual native disassemblies were compared instruction by instruction
for the distinct continuous row **control**:
- After the already recovered row counter increment, emit `ESC E`
  on row 1 of subsequent labels (`label_count != 1`) without checking
  its HRESULT. This check is **independent** of the die-cut path's
  separate medium-switch-flag condition.
- Skip raster content when the current label number exceeds the
  OEM medium's requested-label limit. When row index equals the
  physical page height, reset row index, queued blank rows and the
  cached encoded width (`0xffff`).
- At `row_index > label_height`, reset queued blanks and width cache;
  send `ESC G` and increment the 16-bit label counter **only when**
  `row_index == label_height + 1`. The cutter is therefore delayed
  one raster callback beyond the nominal label length.
- For `IsCustomSize == 0`, the original signed remainder
  `(row_index - 1) % label_height` suppresses the first **14** rows.
  This is not a universal 14-row print margin; it is the exact
  condition in this OEM branch.
- A blank row increments pending blank lines. On the final row of
  a manually configured label (`ModeAuto == 0`) emit `ESC f`
  with that count and clear it. For automatic mode, keep pending
  blank rows. The result of this final-row write is ignored.
- Before a nonblank row, flush pending `ESC f`; clear the pending
  count before checking **exact** `E_FAIL`. On `E_FAIL` stop.
  Other callback returns do not stop it. Call the already recovered
  raster encoder with the counted leading zero bytes, and **forward**
  its result (unlike the other OEM row controller's ignored value).

These are independently tested in new `oem_continuous.[ch]`,
without imposing any artificial physical page length. A zero height
or invalid caller pointer is safely rejected instead of allowing
native division-by-zero or invalid pointer behavior.

**Connected output path:** `oem_continuous_bridge.[ch]` supplies
the existing `oem_row_dispatch` callback for continuous material,
and the existing `raster_stream` code now exports a **single
additive wrapper** `leitz_win2018_stream_encode_nonblank`
around its existing static encoder. No RLE code, last-byte mask,
row-prefix algorithm or die-cut routine has been duplicated or
changed. The combined offline test covers 19 consecutive rows,
the original 14-row skip, blank-line flushing, encoded ink,
`ESC G` and requested-label suppression.

**Limits:** These are disassembly-derived stream vectors and
portable adapters. The original Windows COM/Unidrv entry ABI,
full printer status and physical USB/spooler behavior have not
been executed for parity. Actual hardware cutter timing remains
to be verified on a device. The manufacturer's native 16-bit
row/length and label-count fields are retained, not replaced with
an invented 170-mm length cap.


## Recovered OEM WriteData / COM spool transfer and raster slot 8

A duplicate-work check against branch `sources` at
`e3f98bf489e3eaea55a47091fbbf2b8874a89972` confirmed that the previously recovered GPD,
RLE, die-cut and continuous raster streams and COM class factory
had no native `LeitzRp.dll` WriteData helper model.

**Original Windows binaries, directly inspected:** x86 LeitzRp.dll
RVA `0x29f0`, x64 `0x29e0`. The OEM increments an internal
cumulative requested-byte counter **before** each host-write call.
x86 uses a 32-bit DWORD at object +0x0c, x64 a 64-bit QWORD at +0x18.
The device-spooler interface is called at COM VTable +0x10 (x86)
or +0x20 (x64), receiving the output length as a separate DWORD.
The original returns S_OK if `written == requested` and HRESULT
is **not exactly** `0x80004005`; other negative HRESULTs alone
do not make the original fail. Otherwise the original invokes
COM host VTable +0x28 (x86) or +0x50 (x64) with error code **200**
and returns E_FAIL without undoing the cumulative byte count.

New `oem_write_data.[ch]` retains the original successful/failed
transfer gates and supports both original counter widths; an adapter
matches the existing `leitz_win2018_row_write` signature, so it
can be used by both previously recovered raster streams without
duplicating encoding logic.

**Original VTable slot 8:** at x86 RVA 0x14e0 and x64 RVA 0x14f0,
the OEM checks cumulative bytes against `5 * medium_WORD_0x0c + 16`.
On equality only, it calls the same WriteData helper with the
**single byte 0x1b**. Regardless of that write's outcome, it frees
the nested pointer in a previously allocated block and then the
outer allocation. It returns S_OK. This slot's Unidrv semantic
name is not inferred. New `oem_slot8_finish.[ch]` models the
branch and externalizes the two native frees using callbacks;
it does not manage the Windows heap itself.

New standalone tests cover exact HRESULT comparison, partial
writes, error 200, per-architecture 32/64-bit accumulation,
the conditional one-byte write, and the two cleanup calls.
An integration test uses `oem_write_data` as the existing original-
derived `raster_stream` write callback: three OEM-shaped writes
(4+4+2 bytes) produce `ESC w`, `ESC f`, `ESC G`.
Injecting a short write invokes report 200 but does not modify
the existing row-controller's documented ignored-error behavior.

The proof script `analysis/windows-2018/verify_original_rp_write.py`
was executed with the three actual manufacturer ZIPs. Both
Windows original PE architectures passed 21 direct native opcode
checks, and the original Linux C and macOS Classic raster executable
passed SHA-256 verification. GCC, Clang and ASan/UBSan tests passed
offline. See the separate original-binary evidence report.
These are bounded portable C adapters, not an executable native
Win32 COM driver, real spooler or hardware printer validation.


## Original GDI device metrics at Windows raster-job initialization (2026-10-18)

**Proven new original behavior, not a replacement for prior modules.**
The printer driver uses CreateDCW with UTF-16 driver string `WINSPOOL`,
a caller-supplied printer name and DEVMODE, and a NULL output port.
The manufacturer retrieves GetDeviceCaps values from the new DC
**13 times in this exact order**: `10, 8, 10, 4, 6, 88, 90, 40, 42, 110, 111, 112, 113`.
The first call gives VERTRES / raster height, the second HORZRES /
raster width. The third VERTRES and remaining ten calls are made by
the original's diagnostic path; no paper dimensions or hardware
abilities are inferred from them. DeleteDC is called once before
the already recovered media orientation/label-length calculations,
and its return is not checked.

Direct original locations:
- Windows x86 LeitzRp.dll CreateDCW call RVA `0x18df`, the first
  GDI-selector immediate RVA `0x1963`, DeleteDC at `0x1aa6`,
  original UTF-16 WINSPOOL literal at `0x456c`.
- Windows x64 LeitzRp.dll CreateDCW call RVA `0x192f`, the first
  GDI-selector immediate RVA `0x19a2`, DeleteDC at `0x1b2d`,
  original UTF-16 WINSPOOL literal at `0x46d8`.

New standalone `oem_gdi_caps.[ch]` exposes callbacks for the
native operations and forwards **only measured GDI geometry** to
the already existing `label_setup.c` implementation. It does not change
the physical cassette selection, print length, die-cut/continuous
decision, cutter or RLE encoding, and introduces no length limit.
Native pointer/failed-CreateDC semantics are safeguarded: a missing
DC leaves outputs unchanged; successful DCs get all 13 calls and
one teardown before the prior media initializer executes.
The original would not necessarily validate bad caller arguments;
the portable adapter does.

The new offline fixture test checks the 13 selectors in order,
UTF-16 driver name, NULL port, caller printer and DEVMODE identity,
DC cleanup even on a failed DeleteDC return, untouched state on
CreateDCW failure, normal/custom/landscape dimensions and continuous
manual label length through **the existing** media initializer.
The branch CI builds this test against the real repo's label_setup.c
(not the local compile fixture). GCC, Clang, GCC ASan and UBSan
passed locally.

Verification script:
`analysis/windows-2018/verify_original_rp_gdi_caps.py`
uses only Python stdlib to read all three original ZIPs. It checks
original ZIP and source hashes and compares 13 selector immediates,
CreateDCW and DeleteDC native call bytes and the UTF-16 WINSPOOL
literal in each original Windows PE architecture (26 selectors total).
The original Windows, macOS 5.1.465 and Linux 1.0.3 checks
completed successfully offline. The OEM binaries are intentionally
not stored in GitHub; GitHub CI checks the portable code, not original
Windows-runtime parity.

Remaining: actual native Unidrv COM call ordering and deployment,
real windows spooler and physical label/cutter tests. This
component is **not** a working Windows DLL or Linux CUPS driver.
