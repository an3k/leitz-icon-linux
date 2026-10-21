# Windows LeitzLm 2.17.4.112 — partial language monitor reconstruction

Source: user's archived \`Drivers/amd64/LeitzLm.dll\`, SHA-256
\`8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d\`.
Static disassembly only; no Windows binaries run.

The x64 dispatch entry at RVA 0x1000..0x10a0 checks three
case-sensitive UTF-16 request IDs with the imported \`lstrcmpW\`:
\`leitz_oem_status\` -> ESC A, \`leitz_oem_cartridge\` -> ESC U,
\`leitz_oem_version\` -> ESC V. The recognized branches call
the common worker at RVA 0x1750, store the worker's numerical
response-length result and return TRUE; the unmatched branch
returns FALSE without invoking the worker.

The native worker at RVA 0x1750..0x1851 launches an async
reader, waits 0x1388 (5000 ms), terminates its thread and
copies its accumulated reply. The reader at RVA 0x1860..0x1aee
requests 0x3ff bytes per ReadPort, collects into a 0x400-byte
buffer, and completes on total length == expected response size
or a trailing zero byte. The original's abrupt TerminateThread
and potentially overflowing repeated full-size reads are
**not** adopted. The portable collector has a strict 1024-byte
limit; its callback API is NOT the OEM's Windows monitor ABI.

Source code and tests:

- \`status_dispatch.[ch]\`: original-command dispatch, bounded
  reply collection and explicit safe failure behavior;
- \`status_dispatch_test.c\`: all three queries, exact casing,
  unknown-query no-side-effects, acknowledged query failures,
  segmented response, known-length termination, NUL termination,
  overflow and invalid pointers;
- \`sh test.sh\`: independent GCC and Clang tests.

Missing: original asynchronous event/monitor object ABI, real
response framing and semantics for each command, DLL-native
runtime comparisons, physical communication and error recovery.
Do not issue these commands to the printer until real response
framing and safe device-idle conditions are verified.


## Windows 2018 LeitzLm native MONITOR2 registration and port forwarding

This work extends the original \`LeitzLm.dll\` reconstruction *only where
missing*. The preexisting \`status_dispatch.[ch]\` already models the
three original vendor status requests and bounded asynchronous reply
collection. It is preserved unchanged.

The three user-supplied original manufacturer ZIP archives were directly
read, not executed; their SHA-256 hashes are:

- Linux 1.0.3 \`54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17\`;
- Windows OEM 2018 \`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca\`;
- macOS Classic \`6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210\`.

The actual x86 and x64 original \`LeitzLm.dll\` SHA-256 digests:

- x86 \`4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e\`;
- x64 \`8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d\`.

The original exported \`InitializePrintMonitor2\` is at x86 RVA
\`0x1560\` / x64 RVA \`0x16e0\`. It initializes the caller's returned
monitor handle to NULL, allocates a native monitor object of **12
bytes x86 / 24 bytes x64**, saves the registry/context pointer at
native object **+4/+8**, clears a separate linked-list head at
**+8/+16**, and returns the native MONITOR2 callback table
at .data RVA **0x4000**. In the observed allocation-failure branch
it calls SetLastError(0) before returning NULL. Native allocator
ownership, Win32 thread synchronization and callback calling
conventions remain out of this portable model.

The original callback table has **22 entries** in both architectures,
including the first byte-size field (0x58 x86, 0xb0 x64).
The directly read original native function entries are:

| MONITOR2 slot | OEM function | x86 original RVA | x64 original RVA |
| --- | --- | --- | --- |
| 3 | OpenPortEx | 0x11a0 | 0x12a0 |
| 4 | StartDocPort | 0x12e0 | 0x14b0 |
| 5 | WritePort | 0x1150 | 0x1220 |
| 6 | ReadPort | 0x1080 | 0x1150 |
| 7 | EndDocPort | 0x13d0 | 0x1600 |
| 8 | ClosePort | 0x1040 | 0x1100 |
| 13 | GetPrinterDataFromPort | 0x1440 | 0x1000 |
| 18 | Shutdown | 0x1000 | 0x10b0 |

Other slots are NULL (not a claim about which functions the printer
hardware supports). The x86 status callback is a wrapper at RVA
0x1440, unlike x64 which points directly to its native dispatcher
at 0x1000. **Do not confuse x86 0x1000 Shutdown with x64 0x1000
GetPrinterDataFromPort.** The new
\`monitor2_native_map.[ch]\` captures each exact original slot and the
InitializePrintMonitor2 state setup as a portable, safe data model.

The original **WritePort** callback:
- x86: native monitor port's stored underlying handle +0x0c and
  delegate +0x28; x64: handle +0x18 and delegate +0x50.
- For NULL native monitor port, SetLastError(87) and return FALSE.
- For valid port, invoke the stored delegate exactly once, return
  its BOOL unchanged and do not reinterpret partial byte counts.

The original **ReadPort** callback:
- x86: native handle +0x0c, delegate +0x2c;
  x64: native handle +0x18, delegate +0x58.
- Invoke the stored delegate and return its BOOL directly; the
  OEM does not guard against a NULL monitor port before dereferencing.
- The portable adapter checks invalid pointers and does not
  manufacture a SetLastError call on the ReadPort path.

New \`monitor2_port_io.[ch]\` captures that proven forwarding
behavior via explicit, caller-owned callbacks. On a NULL C
\`port\` there is no available port-local error callback, so it
safely returns FALSE without reproducing the original global
SetLastError(87); invalid inputs with an existing callback report 87.
Native HANDLE ownership, real spooler port open/close and Windows
runtime behavior are **not** emulated.

New offline tests prove all 22 x86/x64 MONITOR2 entries,
initialization success/allocation-failure, WritePort and ReadPort
delegate offsets/forwarded results, transfer lengths, partial
transfers and invalid inputs. \`monitor2_status_integration_test.c\`
calls the **existing** status_dispatch module, forwarding
three manufacturer requests over the new port callbacks:
\`ESC A\`, \`ESC U\`, \`ESC V\`. All pass with GCC, Clang and
GCC AddressSanitizer/UndefinedBehaviorSanitizer.

The independent source verifier
\`analysis/windows-2018/verify_original_lm_monitor2.py\` parses
both ORIGINAL Windows DLL PE tables and checks 22 slots each,
the exact sizes, 13 x86 and 11 x64 direct native instruction
patterns (24 total) and the original SHA-256 for Linux/macOS
raster2printer. All original-source checks passed offline on
the three supplied archives. The OEM ZIPs remain private and
are NOT added to GitHub; CI compiles C regressions without them.

This is not a native Windows language-monitor DLL or executable
Unidrv/Winspool ABI. It does not send physical device commands,
alter existing Linux/CUPS production code, change any stock
format or impose fixed print lengths. Native OpenPortEx/StartDoc/
EndDoc/ClosePort function bodies, real hardware status framing
and physical output remain to be fully reconstructed/verified.


## Windows OEM EndDocPort / ClosePort (new native lifecycle recovery)

Original source preflight: branch \`sources\` at
\`6bff2b326ab4a3f8ca834991cdf0b83218aedca4\`; previous
Windows CI successful. Verified the three manufacturer archives
by SHA-256, Linux 1.0.3 \`raster2printer.c\`, original macOS Classic
\`raster2printer\`, and Windows i386/amd64 \`LeitzLm.dll\`.
Neither manufacturer DLL was executed.

Two **previously missing** native MONITOR2 methods have now been
separately reconstructed as a portable caller-owned callback model
\`monitor2_document_lifecycle.[ch]\`:

| Original MONITOR2 method | i386 RVA | amd64 RVA |
| --- | --- | --- |
| slot 7 EndDocPort | 0x13d0 | 0x1600 |
| slot 8 ClosePort | 0x1040 | 0x1100 |

**Original EndDocPort (both binaries):**
1. Call imported Windows \`SetEvent\` on the port's event handle
   (x86 offset +0x70; x64 +0xe0), ignoring its result.
2. Invoke the original saved host EndDocPort callback at x86 port
   +0x30 / x64 +0x60, passing the original underlying handle
   at x86 +0x0c / x64 +0x18.
3. Iterate the port's linked job list (head x86 +0x08 / x64
   +0x10). For every job, invoke Windows \`SetJobW\` with the
   job's printer handle, job ID, \`Level=0\`, NULL JOB_INFO,
   **literal command 7**. The manufacturer continues on
   SetJobW failure (only prints a diagnostic message).
4. Free every job record, clear the list head using the OEM
   internal helper at x86 RVA 0x1b20 / x64 RVA 0x1e60.
5. Return the **saved original host EndDocPort BOOL** without
   modifying it based on SetEvent, SetJobW or cleanup results.

**Original ClosePort (both binaries):**
- Check the host ClosePort callback at x86 +0x34 / x64 +0x68.
  If present, invoke it on the underlying port handle.
- Regardless of that BOOL or whether a callback was present,
  invoke OEM internal port unlink/release helper at x86
  RVA 0x1a60 / x64 RVA 0x1d30 and return **TRUE**.
- The native release helper also unlinks and frees associated
  internal structures. The portable callback represents this
  ownership boundary but does not reimplement the Windows heap.

The new C module supplies safe external SetEvent, EndDocPort,
SetJobW, job-free, ClosePort and port-free callbacks. It has
explicit validation for missing required delegates instead of
dereferencing wild pointers; the caller must provide an acyclic,
valid job list. It does **not** implement Winspool, Windows
thread/critical-section behavior, COM function-pointer layout,
native printer handles, or physical printer I/O.

The new \`monitor2_document_lifecycle_test.c\` asserts the exact
event/host/enddoc/job notification/job free sequence, two jobs,
literal command 7 with level 0/NULL parameters, original BOOL
return forwarding, ignored failed SetEvent/SetJob results,
optional ClosePort delegate and safe invalid state. GCC, Clang
and AddressSanitizer/UndefinedBehaviorSanitizer passed offline.

Independent original-only verifier
\`analysis/windows-2018/verify_original_lm_document_lifecycle.py\`
rechecks **15 x86 + 15 x64 native instruction byte sequences**
directly from original PE sections in the Windows manufacturer
ZIP, plus the internal Linux and macOS original source checksums.
The original PE/ZIP verifier passed locally. OEM archives are
not added to the public GitHub tree. CI runs the deterministic C
tests alongside every existing language monitor regression.

**Open:** original native OpenPortEx and StartDocPort method bodies,
real Winspool/MONITOR2 registration and end-to-end hardware
spooling/status/automatic-cut validation. The actively deployed
Linux driver and production GitHub branches were not changed.


## Original Windows MONITOR2 OpenPortEx / StartDocPort recovered

**Duplicate-work checkpoint:** Before changing the branch, `sources`
HEAD was `8166a6fba1f4e1b6069c7c3ecb10718dc0130f3e` and the entire Windows CI was green. The earlier
reconstructions of the MONITOR2 22-entry table, WritePort, ReadPort,
GetPrinterDataFromPort, EndDocPort, ClosePort and raster data encoder
were read and are unchanged. Only the previously missing MONITOR2
slots **3** and **4** are addressed here.

Original manufacturer archives re-opened, independently SHA-256
matched against the recorded original sources:

- Linux `LeitzIconDriversForLinux1.0.3.zip`:
  `54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17`
- Windows `LeitzIconLabelSetup.zip`:
  `cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`
- macOS `LeitzIconSoftware-Installer_5.1.465.dmg.zip`:
  `6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210`

Original Windows LeitzLm.dll x86 SHA-256
`4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e`,
x64 SHA-256
`8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d`.

| Original native routine | x86 RVA | x64 RVA |
| --- | --- | --- |
| MONITOR2 slot 3 OpenPortEx | `0x11a0` | `0x12a0` |
| MONITOR2 slot 4 StartDocPort | `0x12e0` | `0x14b0` |

**OpenPortEx**, new `monitor2_open_port.[ch]`:
- Check the supplied MONITOR2 host callback table at five required
  pointer slots **2, 4, 5, 6, 8** (x86 offsets 8/16/20/24/32,
  x64 offsets 16/32/40/48/64). Missing callback gives
  `SetLastError(3007)` and FALSE.
- Under the OEM global synchronization, check the port name against
  already registered ports. Duplicate gives `SetLastError(170)`
  and FALSE.
- Allocate and register the native port, which the original
  helper creates with **0x478 bytes x86 / 0x4f0 bytes x64**.
  The callback-driven portable allocator owns that registration;
  no Windows heap or global linked list is fabricated.
- Call the supplied host OpenPort function at x86 callback +8 /
  x64 callback +16, passing the underlying handle output at
  native port x86 +0x0c / x64 +0x18.
- On failed host opening, invoke the existing conceptual port
  unlink/release boundary without publishing a port. On success,
  copy **22 pointer entries** (88/176 bytes in the original)
  into its stored host table, publish the port and report success.
  This model uses `uintptr_t` entries and callbacks rather than
  pretending that native x86/x64 function pointers are portable.

**StartDocPort**, new `monitor2_start_doc.[ch]`:
- Validate port pointer, nonempty bounded UTF-16 printer name and
  required fifth document-info parameter. Invalid native arguments
  produce `SetLastError(87)` and FALSE.
- Invoke `WaitForSingleObject(event, 60000)`, then
  `ResetEvent(event)`, ignoring both results as in the OEM.
- Allocate a record for the UTF-16 name (the manufacturer's
  allocation expression is **2*name_length+0x12** on x86 and
  **2*name_length+0x22** on x64, including terminator).
  The portable allocator receives the name and bounded character
  count and must own its name copy; no Windows allocator is cloned.
- Call `OpenPrinterW` to obtain a printer handle, store the
  supplied JobId and prepend the new record to the existing
  per-port job list (native head x86 +8/x64 +16).
- Finally invoke the stored host `StartDocPort` callback (x86
  port +0x24 / x64 +0x48), forwarding its BOOL unchanged.
  The record remains in the list even if this host callback returns
  FALSE, reflecting the actual native control flow.
- On unsuccessful `OpenPrinterW`, the safe portable model frees
  the **unlinked** record. The original has a less safe early
  return path and may leave this allocation behind. This is an
  explicit memory-safety difference, not claimed byte-for-byte ABI.

Regression coverage: `monitor2_open_port_test.c` verifies
required host slots, duplicate errors, lock/lookup/open order,
successful table copy, failed host opening and cleanup.
`monitor2_start_doc_test.c` verifies event wait/reset, allocated
job list, failure result retention, and bad arguments.
`monitor2_start_doc_integration_test.c` calls the **existing**
`monitor2_document_lifecycle.c` to complete a job and check
`SetJobW(Level=0,NULL,7)` and list cleanup, without changing
its source. GCC, Clang, GCC ASan/UBSan passed locally for
both independent and integration tests.

Primary-source verifier
`analysis/windows-2018/verify_original_lm_open_start.py`
checks **25 x86 and 24 x64** direct original machine-code
sequences (49 total), both original Windows DLL SHA-256 hashes,
all three unchanged OEM archives and original Linux/macOS
raster filter hashes. All offline checks passed. Original vendor
binaries are NOT checked into the repository.

**Limit:** This is an independently testable native-behavior
reconstruction with caller-owned callbacks, not a compiled native
Windows MONITOR2 DLL or a tested physical Linux driver. Real
Windows host, network/USB printer I/O, cutter and monitor-thread
behavior have not been validated against actual device traces.


## Native MONITOR2 shared port release and Shutdown (new)

Before this change, `sources` was at `5cc088752a0ce08706e84859a4342ef7181332ef` and the full
Windows static reconstruction CI passed. Previously recovered
`OpenPortEx`, `StartDocPort`, `WritePort`, `ReadPort`,
`EndDocPort` and `ClosePort` remain untouched. The missing
common **native port-destructor** behavior and **Shutdown**
traversal are now reconstructed separately.

Original OEM LeitzLm.dll directly examined in both architectures:

| OEM routine | Original i386 RVA | Original amd64 RVA |
| --- | --- | --- |
| Shared port destruction helper | `0x1a60` | `0x1d30` |
| MONITOR2 Shutdown (table slot 18) | `0x1000` | `0x10b0` |
| Imported `DeletePrinter` thunk | `0x1b70` | `0x1ec2` |

The destructor:
1. Unlinks a port from the OEM's global port linked list
   (original `next` offset i386 +4, amd64 +8).
2. If the port's recorded printer handle at i386 +0x10 /
   amd64 +0x20 is non-NULL, calls the **WINSPOOL.DRV
   `DeletePrinter` export**, ignoring its BOOL return.
   This is **not `ClosePrinter`**, and it may remove a
   configured Windows printer. The exact imported function
   was confirmed in the original DLL import tables and
   jump thunks.
3. If the event handle at i386 +0x70 / amd64 +0xe0 exists,
   calls `CloseHandle` and ignores its BOOL.
4. Under the job-list critical section, frees the port's
   linked job nodes, sets the job-list head to NULL and
   releases the critical section. Original job-list head:
   i386 +8, amd64 +16.
5. Calls `GlobalFree` on the name allocation at port +0
   and then on the port object itself.

The **Shutdown** callback gets the monitor's current port
list (i386 monitor +8, amd64 +16), saves each node's next
pointer before destruction, calls the **same shared helper**
for every port, and finally calls `GlobalFree` on the
monitor allocation. It does **not** call the saved host
`ClosePort` delegate as a separate step.

New `monitor2_owned_cleanup.[ch]` models these distinct
ownership transitions using caller-supplied safe callbacks
for `DeletePrinter`, `CloseHandle`, lock/unlock and
`GlobalFree`. It cannot call the destructive Windows
`DeletePrinter` itself. Do **not** bind that callback to a
real printer until the OEM reason for doing so and native
handle ownership are independently validated. The model
does not allocate or dereference Windows HANDLEs or emulate
native critical-section semantics.

`monitor2_owned_cleanup_test.c` checks unlinking,
conditional original import callbacks, precise delete /
close / job-lock / GlobalFree ordering, shutdown
traversal and invalid input. Integration test
`monitor2_owned_cleanup_integration_test.c` passes
this same reconstructed destructor into the **unchanged**
`monitor2_document_lifecycle.c` ClosePort callback to
check the original host ClosePort -> native release sequence.
GCC/Clang strict C11 and GCC ASan/UBSan passed locally.

A new independent original-only verifier
`analysis/windows-2018/verify_original_lm_owned_cleanup.py`
reads the three original manufacturer ZIPs, verifies their
SHA-256 hashes, checks original LeitzLm.dll i386/amd64 hashes,
**20 x86 and 21 x64 native instruction patterns** including
the DeletePrinter jump thunks, and verifies the original Linux
1.0.3 and macOS Classic raster filter hashes. All pass offline.
No OEM DLL is executed, redistributed or embedded in GitHub.

**Not complete:** Native Windows monitor ABI, actual memory
allocator/locking behavior and physical printing/cutting
remain untested. The portable safe model does not endorse
the original's potentially destructive printer deletion.


## LeitzLm original native port registration and lookup (new)

**Duplicate-work preflight:** On `sources` HEAD `c877eb3a229a96cdb037c99c5765bce9fef57887`,
the previous Windows CI was successful. Existing MONITOR2 callback
map, OpenPortEx, StartDocPort, port I/O, EndDocPort/ClosePort,
status dispatch and shared owned-port destructor were already
reconstructed and have **not been replaced**. This addition
recovers their formerly abstracted **native registry lookup and
port allocation** helpers.

Manufacturer `LeitzLm.dll` originals were read from the unchanged
user-provided Windows ZIP (SHA-256
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`).
Both i386 and amd64 DLLs matched their documented individual hashes.
Linux 1.0.3 and macOS Classic manufacturer archives and
`raster2printer` source hashes were also rechecked.

| Original helper | i386 RVA | amd64 RVA |
| --- | --- | --- |
| Case-insensitive lookup of registered port name | `0x1ae0` | `0x1e00` |
| Allocate/register a port with Win32 resources | `0x19b0` | `0x1c40` |

**Exact original lookup behavior:** Reject NULL/empty UTF-16
input, traverse the global singly linked port list in order,
compare each saved port name using imported Windows
**`lstrcmpiW`**, return first matching port, or NULL.
Names are not compared bytewise or by ASCII-only case folding.
The new portable module calls an externally supplied comparison
function instead of incorrectly claiming to reproduce
Windows Unicode collation.

**Exact original port-construction behavior:**
- `GlobalAlloc` a native port block of **0x478 bytes on i386**
  or **0x4f0 bytes on amd64**, then zero the allocated block.
- Allocate and copy the UTF-16 name including its terminating
  NUL as a **separate allocation**.
- Set its global next pointer to the prior head and register
  the new port **before** creating the event.
- Clear printer handle, job-list pointer and the OEM state flag.
- Call `CreateEventW(NULL, TRUE, TRUE, NULL)`: manual-reset and
  **initially signaled**. Store the returned event handle.
- Initialize a separate **0x400-byte port scratch region** to zero.
  The original offsets for the event and scratch are
  i386 `+0x70/+0x74`; amd64 `+0xe0/+0xe8`.
- The OEM does **not** reject a NULL `CreateEventW` result after
  registering the port. The new model preserves that behavior
  explicitly and reports the NULL event in its state.

New `monitor2_port_registry.[ch]` uses the existing
`leitz_lm_owned_port` record as its leading field, so the
**existing** `monitor2_owned_cleanup.c` unlinks and frees
the new port and separate name allocation; no second destructor
is written. This is a portable state model, not binary-compatible
Win32 layout. Allocation and event creation are external callback
boundaries; none invokes native spooler APIs.

**Deliberate safety correction:** On allocation failure for the
native port or name, the manufacturer's path can dereference a
NULL port pointer. The portable implementation safely returns
without registering a broken entry, and frees any newly allocated
unlinked port. It also validates the caller's bounded UTF-16
name and integer capacity. Registry iteration requires an acyclic
caller-owned list, and its original external OpenPortEx caller
must hold the synchronization lock.

The independent regression `monitor2_port_registry_test.c` checks
both native allocation sizes, UTF-16 name cloning, case-insensitive
lookup callback, duplicate and absent search, insertion order,
flag and 1024-byte scratch initialization, manual-reset/signaled
event parameters, permitted event failure, allocation-failure
cleanup and actual integration with the previously recovered
port destructor. Strict C11 GCC, Clang and GCC ASan/UBSan pass.

The new standalone source verifier
`analysis/windows-2018/verify_original_lm_port_registry.py`
reopens the original manufacturer ZIPs, verifies their full
SHA-256 values, compares **21 i386 and 20 amd64** directly
transcribed machine instructions in the actual LeitzLm PE
sections, proves the native `lstrcmpiW` and `CreateEventW`
import names and rechecks the unchanged Linux/macOS filters.
All original source checks pass offline. The manufacturer
DLLs are not added to GitHub.

**Open:** Binding these portable submodules to a fully executable
native Windows monitor and validating actual Windows spooler
port concurrency, real printer status, printed output and
automatic hardware cutting.


## Existing MONITOR2 registry -> OpenPortEx -> destructor integration

Preflight checked `sources` commit
`57de89b8a40708949a2076a1baca5cced6c6e12f` and its successful Windows reconstruction CI.
The new `monitor2_registry_open_integration_test.c` uses the
**unchanged, real C implementations**:
`monitor2_port_registry.c`, `monitor2_open_port.c` and
`monitor2_owned_cleanup.c` in the same test binary.

The cross-module regression checks that:

- Port opening invokes native registry allocation (0x4f0-byte
  x64 layout), clones its UTF-16 name, initializes its 0x400-byte
  scratch area, registers it and creates a manual-reset/signaled event.
- The existing host OpenPort callback and its saved 22-entry
  table produce a usable separately modeled OpenPortEx view.
- A case-variant spelling of a registered port is rejected
  with error 170 **before** another host-open or allocation.
  The portable fixture's comparison handles only ASCII test
  names: it does **not** claim equivalence to Windows lstrcmpiW
  for arbitrary Unicode.
- Port release delegates to the **preexisting** OEM-derived
  destructor, unregisters the port, closes its event and frees
  the separate UTF-16 name and port allocations.
- Failed host OpenPort releases its newly registered allocation
  without publishing a port pointer; absent required host
  callbacks are rejected with error 3007.
- **No actual Windows DeletePrinter API is used.** The
  DeletePrinter callback is a test-only guard and must not
  be connected to real printer deletion without resolving
  native handle ownership and side effects.

Strict C11 GCC, Clang and Clang AddressSanitizer/UndefinedBehaviorSanitizer
completed successfully with local interface-equivalent copies of
the earlier source modules. Repository CI compiles this test
against the actual unchanged upstream modules. No new
production module, Windows binary, or real spooler integration
is implied by this test.

The original 2018 Windows, Linux 1.0.3 and macOS 5.1.465 OEM ZIPs
were re-read; their full SHA-256 hashes match the previously
recorded originals, as do the Windows LeitzLm.dll and the
Linux/macOS raster2printer source bytes.

**Still open:** a native Windows MONITOR2 binary/API shim,
synchronization/lifetime under real Winspool concurrency,
true Win32/USB I/O and physical print/cut parity.


## Cross-module full-session monitor regression (2026-10-21)

**Preflight/deduplication:** Verified `sources` at `afa78b35d28745a94d92a1b5b8003443a4107d2b`
with passing Windows CI, after concurrent addition of the separate
`monitor2_registry_open_integration_test.c`. That earlier test
already covers registry -> OpenPortEx -> cleanup, so those functions
were NOT rewritten. This new check extends the chain across
**port data I/O, status, StartDocPort, EndDocPort and ClosePort**
using seven preexisting implementation modules, in both modeled
x86 and x64 allocation variants.

New `monitor2_full_session_test.c` compiles the real existing
`monitor2_port_registry.c`, `monitor2_open_port.c`,
`monitor2_port_io.c`, `status_dispatch.c`,
`monitor2_start_doc.c`, `monitor2_document_lifecycle.c`
and `monitor2_owned_cleanup.c`. A deterministic callback fixture:

- opens a registered named port and verifies original event
  initialization, 22 host callback entries and both architecture
  allocation sizes;
- rejects differently-cased duplicate port names (Win32 error 170);
- writes exact data bytes, then sends the original vendor status
  command ESC A via the ReadPort/WritePort callback pair;
- starts an actual modeled job with a 60000-ms wait/reset, saves
  printer handle and calls host StartDocPort;
- completes it with original SetJobW(job, level 0, NULL, command 7)
  and frees the job despite failed SetJob results;
- calls host ClosePort and the existing owned destructor, ensuring
  monitor registry unlink, event close, balanced locks and no
  outstanding native name/port allocations.

The test passed strict GCC, Clang, and Clang ASan/UBSan locally
with functionally matching source fixtures; the branch CI executes
it against actual repository sources. Previous individual and
registry-open integration tests remain unchanged.

**Important boundary:** the independently reconstructed
`document_port.jobs` and `owned_port.jobs` types are NOT
a unified native Windows object. The fixture guarantees that
the document list has been completed before the owned destructor
runs. The new test does not claim native MONITOR2/Unidrv ABI,
actual Windows spooler execution, a physical print, or a
validated cutter response. Completing those remains necessary.
The original Linux 1.0.3, Windows OEM 2018 and macOS Classic
ZIP SHA-256 values and the x86/x64 LeitzLm DLL hashes were
rechecked unchanged before this work.


## Shared native job-list ownership across existing MONITOR2 modules

**Duplicate-work preflight:** `sources` at `8367271ff0529298a70024b8f212008aff657d96`, full
Windows CI passed. The existing registry, StartDocPort, EndDocPort,
ClosePort, owned-port destructor and full-session tests were reviewed
and preserved without replacement. Exact original manufacturer
Linux 1.0.3, Windows 2018 and macOS Classic 5.1.465 archive
SHA-256 digests match the established inventory.

The native OEM language monitor uses **one job-list head per port**,
at x86 +0x08 / x64 +0x10, shared by StartDocPort, EndDocPort,
its list-clearing helper and the port destructor. The prior
independent portable `leitz_lm_document_port.jobs` and
`leitz_lm_owned_port.jobs` structures expose **two views**, a
gap the full-session fixture previously sidestepped by finishing
all jobs before cleanup. This new work addresses that gap directly.

New isolated `monitor2_job_owner_bridge.[ch]` wraps the **existing**
functions without rewriting them:
- Verify both list views match **before any side effects**; reject
  mismatched ownership or already-closed bridge with -1.
- On StartDocPort synchronize the owned list even when the host
  StartDocPort returns FALSE **after registering a job**, as in OEM.
- On EndDocPort synchronize the owned list after the existing code
  sends SetJobW(level 0, NULL, command 7) and frees its entries.
- On ClosePort clear the non-owning document view before calling
  its existing `release_port` callback, which must invoke the
  existing `leitz_lm_release_owned_port` destructor. That destructor
  solely owns outstanding jobs; mark bridge closed after release.
  Missing destructor callback or out-of-sync views cause no
  side effects. The document view itself remains caller-owned.

`monitor2_job_owner_bridge_test.c` checks **two completed jobs**,
their original SetJobW notifications, no double frees; a failed
host StartDocPort leaving a job registered until ClosePort;
mismatched list rejection without mutation; missing cleanup
callback; closed-bridge non-reusability; and resource counts.
Strict GCC and Clang C11, and Clang ASan/UBSan passed offline
against matching existing source interfaces. The branch CI builds
the test against the **actual unchanged repository modules**.

Original manufacturer verification:
`analysis/windows-2018/verify_original_lm_shared_job_owner.py`
was run on the actual three original ZIPs. It confirmed
5 x86 + 7 x64 opcode sequences identifying the *same*
per-port list head in StartDocPort, EndDocPort, list clearer,
and the x64 owned-port destructor; also checked original
Windows LeitzLm binary SHA-256 plus Linux/macOS native filter
checksums. Originals are not checked into the repository.

**Scope limitation:** the bridge is a portable, caller-serialized
source-level ownership mechanism. It is **not** the native
MONITOR2 memory layout, lock implementation, Win32 ABI,
a physical printer test or proof of automatic-cut parity.
The callbacks for release ownership must be supplied correctly;
no real DeletePrinter or device commands are invoked by tests.
