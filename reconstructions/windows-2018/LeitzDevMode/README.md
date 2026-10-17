# Windows LeitzDevMode.dll 2.17.4.112: native function reconstruction

**Original binaries from the uploaded manufacturer Windows ZIP:**
- x86: `LeitzDevMode.dll` SHA-256
  `fcb83126f09d7c51dcccc78cef6869c0d0fdc60f7b779b51304cf00e95ea5c99`
- x64: `LeitzDevMode.dll` SHA-256
  `b5bd1b6c0c4a94bcc8747dd3941283537d353517f3a445a8397130f06def5342`

Both export **`hrOEMDevMode` at RVA 0x1080**. Original x86 and x64
disassemblies agree on four operations: 1=SIZE, 2=DEFAULT, 3=CONVERT,
4=MERGE, with separate parameter-buffer offsets for Windows x86/x64.
The caller supplies `OEMDMPARAM`; the recovered C API is normalized,
**not** an original Unidrv function signature.

At original x64 RVA 0x10e1 (x86 0x10b5) mode 1 publishes size **28**.
Mode 2 writes exactly five little-endian DWORDs (first 20 of 28 bytes):
size 28, signature 0x4c504944 (literal bytes `DIPL`), DWORD 1, DWORD 1,
version 0x00010000. The trailing eight bytes are **not initialized**.
This follows actual instructions; a memset of the whole structure would
change OEM behavior.

Modes 3/4 call the same native helper at RVA 0x1000. It compares DWORD
signature at +4 between source/destination. If equal, it copies
`min(source.size,destination.size)` bytes, then sets destination size
28 and destination DWORD at +8 to 1. Any matching signature is accepted;
the native helper does **not** require literal `DIPL` at this step.
If signatures differ, it invokes OutputDebugStringW with
`Unknown signature` and initializes the first 20 bytes to defaults.
The exported wrapper returns zero for supported modes (it ignores the
helper's boolean return). Invalid mode/null OEMDMPARAM calls
SetLastError(87) and returns 0x80004005.

New `oem_devmode.[ch]` retains these field writes, decisions, four
operations and optional debug/Win32-error callbacks, but applies
explicit source/destination capacity checks and defined overlapping
memory behavior. This is an intentional safety improvement, not byte-
for-byte ABI parity. The original 28-byte field offsets remain
unmodified and the wider PrintHelper `devmode_patch` is **not**
duplicated or replaced.

Offline `sh test.sh`: GCC/Clang strict C11 tests, plus local GCC
AddressSanitizer/UndefinedBehaviorSanitizer pass. Native DLLs were
disassembled, **not executed**. Original Windows spooler/Unidrv
parameter wiring, real application behavior and device printing remain
outside this isolated recovered function.
