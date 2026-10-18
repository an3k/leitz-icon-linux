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
