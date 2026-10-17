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
