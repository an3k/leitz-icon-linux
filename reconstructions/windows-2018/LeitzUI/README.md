# LeitzUI.dll (Windows 2018): new native-code recovery

Original manufacturer's Windows ZIP (directly uploaded again):
- x86 LeitzUI.dll: SHA-256 `b6f066d87ac18aff5d37c6b4ac0c3c13d6e0cd58bb2afb5fb74152e3e99a5293`
- x64 LeitzUI.dll: SHA-256 `7d514bd5bb4c7610569c0aacd7b37cc74318e5fce858bf4fadba9bb554de0a10`

Both original binaries were inspected with `objdump -d -M intel`; neither DLL was executed. Prior `sources` already contained GPD, LeitzRp, LeitzLm, PrintHelper, LeitzDevMode and WiFiHTTP reconstructions. **There was no existing LeitzUI reconstruction.**

## OEM UI GetInfo

Proven x86 RVA **0x1140** and x64 RVA **0x1150** share:

| Native mode | Required size | Result | Error code |
| --- | ---: | --- | --- |
| 1 | 4 | 4-byte value `DIPL` (DWORD `0x4c504944`) | none |
| 3 | 4 | 4-byte value 1 | none |
| 4 | initially 4, then 0 if output is sufficient | `E_FAIL` | 50 (`ERROR_NOT_SUPPORTED`) |
| 2 or other invalid | not written | `E_FAIL` | 87 (`ERROR_INVALID_PARAMETER`) |
| 1/3/4 with absent/short result buffer | 4 | `E_FAIL` | 122 (`ERROR_INSUFFICIENT_BUFFER`) |

If the required-size pointer is absent, error 87 is returned. The
reconstructed function accepts scalar buffers and an optional
SetLastError callback rather than an unproven native Unidrv ABI.

## CutAfterEachLabel property UI callbacks

Original x86:
- printer UI callback RVA **0x1770**, document UI callback RVA **0x17a0**;
- native private `CutAfterEachLabel` field at **+0x10**, UI
  option item selection at **+0x10**.

Original x64:
- printer UI callback RVA **0x17d0**, document UI callback RVA **0x1820**;
- native private `CutAfterEachLabel` field at **+0x18**, UI
  option item selection at **+0x10**.

The private-field offset differs by native object layout and
**must not be applied to the wrong ABI**. This portable model uses a
normalized state struct rather than either native pointer layout.

- printer reason **6**: copy private low 16 bits to the UI selection;
  always return 0.
- document reason **6**: same reset of UI selection; return 0.
- document reason **10**, original
  `CPSUICB_REASON_APPLYNOW`: compare UI selection with
  private low WORD; if different, write the new selection as a DWORD
  into the private field and OR the OEM flag DWORD with **0x8**,
  returning **1**; otherwise no mutation and return **0**.
- document reason **11**, original
  `CPSUICB_REASON_KILLACTIVE`: restore UI selection from
  private low WORD; return 0.
- other documented event values: return 0 unchanged.

These branches were checked against both original disassemblies.
Only the **UI/private DEVMODE synchronization** is implemented here.
It does not specify when the physical cutter actuates: that depends
on the printing path, which has separately recovered ESC G/E logic.

## Offline verification and scope

`sh test.sh` builds with GCC and Clang in strict C11
and tests GetInfo success/error outputs, 4-byte capacity boundaries,
all 6/10/11 UI event transitions, repeated/no-change apply,
private low-WORD semantics and the 0x8 changed bit. GCC ASan and
UBSan were also run locally. The resulting source blobs match the
locally compiled files by exact Git blob SHA-1.

**Not recovered yet:** native Windows class factory registration and
full OEMUI object ABI, OEM UI dialog construction, COM dispatch,
UI printing on Windows, actual USB data and OEM-runtime output parity.
No changes to manufacturer files, productive Linux code or device.


## Original LeitzUI COM class factory: next recovered dispatch boundaries

The 2026-10-17 preflight found no existing COM factory module at sources
HEAD `3beeaa939aba275f945e18596f944c40e68c4005`;
prior OEM UI GetInfo and CutAfterEachLabel callbacks remain untouched.
The exact original OEM DLL binaries were extracted again from the
user-provided `LeitzIconLabelSetup(5).zip` (SHA-256
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`).
The separately checked original Linux archive and macOS Classic ZIP
were not re-reconstructed, as their original source/finding records
already exist. The Windows DLLs were disassembled offline and never
executed.

Original LeitzUI.dll:
- x86 SHA-256 `b6f066d87ac18aff5d37c6b4ac0c3c13d6e0cd58bb2afb5fb74152e3e99a5293`
- x64 SHA-256 `7d514bd5bb4c7610569c0aacd7b37cc74318e5fce858bf4fadba9bb554de0a10`

The code in `ui_com_factory.[ch]` is a **new portable,
bounded decision-model API** recovered from those DLLs. It is NOT
a DLL exporting real Windows IClassFactory function pointers, a
Microsoft COM registration helper or a complete IPrintOemUI object.

| Original function / decision | x86 RVA | x64 RVA | Native behavior recovered |
| --- | ---: | ---: | --- |
| DLL export DllGetClassObject | 0x1460 | 0x1400 | Accept one exact class UUID; set output NULL first; allocate factory, QueryInterface, Release |
| Class factory QueryInterface | 0x12e0 | 0x1280 | Accept IUnknown and IClassFactory only |
| IClassFactory::CreateInstance | 0x1390 | 0x1310 | Refuse non-null outer instance (NOAGGREGATION); allocate OEM UI, QueryInterface, Release |
| OEM UI QueryInterface | 0x1010 | 0x1010 | Accept IUnknown and one original OEM UI interface UUID |
| IClassFactory::LockServer | 0x1410 | 0x13c0 | Interlocked inc/dec of DLL lock counter |
| DLL export DllCanUnloadNow | 0x1440 | 0x13e0 | S_OK only when two distinct DLL counters are zero; otherwise S_FALSE |

GUIDs recovered from the x86 .rdata and independently compared with
the x64 .rdata (all names below are for readability; the C module
stores actual little-endian Windows GUID bytes):

- class UUID: `abce80d7-9f46-11d1-882a-00c04fb961ec`
- IUnknown: `00000000-0000-0000-c000-000000000046`
- IClassFactory: `00000001-0000-0000-c000-000000000046`
- OEM UI object interface UUID:
  `c6a7a9d0-774c-11d1-947f-00a0c90640b8`

The actual return codes are `S_OK=0`, `S_FALSE=1`,
`E_POINTER=0x80004003`,
`E_NOINTERFACE=0x80004002`,
`CLASS_E_CLASSNOTAVAILABLE=0x80040111`,
`CLASS_E_NOAGGREGATION=0x80040110`,
and `E_OUTOFMEMORY=0x8007000e`.
Factory and UI object initial reference count is 1 in the native
constructor before the internal QueryInterface + Release sequence.
Native factory allocations are 8 bytes on x86 / 16 on x64; OEM
instance allocations are 12 / 24 bytes. These native objects and
their vtables are **not** allocated by the portable model.

The C API receives an explicit allocation-success flag and reports an
abstract resulting interface rather than creating real COM objects.
Two DLL global counts are supplied explicitly to the unload decision.
NULL GUID pointers are safely rejected by the model, although the
original native function often dereferences them unchecked. The
counter lifetime during native object creation is **not** claimed
recovered from these decisions alone.

Test coverage: all four exact GUID byte arrays, successful/unknown
class and interface selection, factory allocation failure,
CreateInstance aggregation rejection, UI IID vs factory IID,
NULL/invalid requests, LockServer transitions, and both
DllCanUnloadNow counters. Local **GCC + Clang + AddressSanitizer +
UndefinedBehaviorSanitizer** passed under strict C11 flags. The same
unchanged LeitzUI CI step now compiles both previously completed UI
callback tests and the new class factory tests.

**Still open:** native Windows COM vtable ABI implementation,
actual registered CLSID activation and object lifetime, all
IPrintOemUI interface methods/dialog setup, Windows runtime output
parity, as well as USB printing and physical label cutting.


## Original property-sheet construction: OEM UI printer/document options

This work **extends**, without modifying, the previously reconstructed
`LeitzUI` callback decisions and COM class factory.
Preflight: sources HEAD `0255d6aa077febea7a5bfa4277beeb857d1c4e0d`,
passing CI, no original property-sheet constructor present.

Actual uploaded OEM Windows 2018 DLLs were reexamined, with SHA-256:
x86 `b6f066d87ac18aff5d37c6b4ac0c3c13d6e0cd58bb2afb5fb74152e3e99a5293`,
x64 `7d514bd5bb4c7610569c0aacd7b37cc74318e5fce858bf4fadba9bb554de0a10`.
The Linux 1.0.3 C original, original Windows GPD, and macOS Classic
original raster2printer Mach-O were also checked against their
previously recorded SHA-256. No installer or OEM executable was run.

The native IPrintOemUI property-sheet selector dispatch is
**mode 1 = document** and **mode 2 = printer** (x86 RVA
0x11f0-0x1287; x64 RVA 0x1200-0x126b).
Invalid mode invokes SetLastError(87), returning E_FAIL.
The property sheet constructors are x86 document RVA 0x1500,
printer RVA 0x1630; x64 document RVA 0x14c0, printer RVA 0x1640.

The original receives different native structures based on ABI:

| Exact observed field | 32-bit DLL | 64-bit DLL |
| --- | --- | --- |
| pointer to OPTITEM array | parameter +0x2c | parameter +0x58 |
| cOptItem | parameter +0x30 | parameter +0x60 |
| property-sheet callback | parameter +0x38 | parameter +0x70 |
| single OPTITEM byte size (cbSize) | 0x34 | 0x58 |
| OPTITEM zeroed byte offset | +0x20 | +0x34 |
| first-item initial selection DWORD | +0x10 | +0x18 |
| OPTTYPE allocation/size | 0x20 | 0x30 |
| OPTPARAM allocation/size | 0x18 | 0x30 |

**Exact shared OEM sequence:** if pOptItem is NULL, write cOptItem=1
and return 0 without allocating anything; otherwise zero
cOptItem × option-item-stride bytes, write cbSize for each item,
set the first item's byte +2 to 1 and DWORD flags +4 to **0x0c**
(document) or **0x04** (printer). Initialize its choice DWORD
with a zero-extended WORD read from the supplied source record
at +0x10 (or 0 if that record is absent). Set one-option
OPTTYPE's cbSize, parameter count=1, an original option type
byte value of **9**, and one OPTPARAM's cbSize; the OEM
assigns allocated pointers, loads localized strings, and registers
the callbacks previously reconstructed at x86 0x17a0/0x1770 and
x64 0x1820/0x17d0. These native pointer assignments are **not**
fabricated inside a portable structure.

New `ui_property_sheets.[ch]` is a bounded C model
of only the **proven data and first option initialization**.
It exposes separate x86 and x64 item sizes, a normalized caller-
provided initial WORD, two callback identities, an independent
OPTTYPE+OPTPARAM serializer with zero pointer fields and no
Win32 memory ownership. Any buffer shortfall or zero item count
is rejected before modification; original code can dereference
invalid native pointers and such behavior is deliberately NOT copied.

Offline GCC/Clang strict C11 and GCC ASan/UBSan passed. Tests cover
both printer/document modes, both architectures, count-only
operation, 3-item initialization, exact buffer boundaries,
initial selection, type/param sizes, canary bytes, NULL cases,
invalid arch/mode, arithmetic overflow and deterministic failures.
The existing `LeitzUI/test.sh` is extended to test this
alongside its old callback and COM factory modules. No changes to
Linux/CUPS production code or original binaries, no physical
printer use. Actual Windows property dialogs and real Unidrv COM
pointer callbacks remain unimplemented.


## Original IPrintOemUI driver-interface publication and cleanup

**New, previously unreconstructed** portable model:
`ui_driver_interface.[ch]`, validated against the original
Windows **x86 and x64** LeitzUI.dll binaries. Both versions contain
the same acquisition routine at **RVA 0x1100**:
- x86 LeitzUI.dll original SHA-256
  `b6f066d87ac18aff5d37c6b4ac0c3c13d6e0cd58bb2afb5fb74152e3e99a5293`
- x64 LeitzUI.dll original SHA-256
  `7d514bd5bb4c7610569c0aacd7b37cc74318e5fce858bf4fadba9bb554de0a10`

The original calls the supplied unknown object's **vtable slot 0**
(`IUnknown::QueryInterface`) with this exact GUID:

`92b05d50-78bc-11d1-9480-00a0c90640b8`

Native little-endian GUID bytes:
`50 5D B0 92 BC 78 D1 11 94 80 00 A0 C9 06 40 B8`
(x86 original RVA `0x313c`, x64 original RVA `0x3210`).
The interface is stored in the object at **+0x08 on x86** and
**+0x10 on x64**. If that slot is already non-NULL, the
routine returns S_OK **without invoking QueryInterface again**.
Otherwise it invokes QueryInterface, accepts any **nonnegative**
signed HRESULT, and caches the returned interface pointer.
For a **negative HRESULT**, it clears the pointer and returns
**E_FAIL (0x80004005)** rather than the original QueryInterface
HRESULT. Even a nonnegative result with a NULL output pointer
causes S_OK; the next call can query again.

The object's final Release/destruction path (original x86 RVA
`0x10a0`, x64 RVA `0x1080`) invokes the cached
interface's **IUnknown::Release** vtable slot and clears the
cache. The original also frees the OEM object itself and may
assert its reference count. Those native lifetime details are
**not** simulated by this new, caller-owned binding object.

The new C interface uses explicit query/release callbacks and
preserves the observed one-time query, HRESULT mapping, cache
transition and cleanup. It deliberately rejects NULL callbacks
rather than triggering the original's unchecked pointer
dereferences. It is **not** the full Windows IPrintOemUI
binary interface; calling convention, actual COM object ownership,
COM reference counts, Unidrv invocation and real UI/device behavior
still require integration.

Offline checks: strict GCC/Clang C11 (Werror), AddressSanitizer,
UndefinedBehaviorSanitizer, correct GUID, skip-on-existing,
negative HRESULT mapping, NULL result on success, repeated release,
error preflight and callback counts. The existing LeitzUI
test script and CI now compile/run the new module alongside the
earlier UI callback, class factory and property-sheet tests,
without modifying any preexisting C reconstruction.
