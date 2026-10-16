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
