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
