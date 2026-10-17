# Windows 2018 original GPD command-byte reconstruction

This directory recovers only **four byte sequences already documented
from the manufacturer's `Drivers/LeitzIcon.gpd`**, not a new print
backend or the Windows `Unidrv` implementation.

| Original GPD field or option | Recovered literal bytes |
| --- | --- |
| `CmdStartDoc` | 121 `ESC` bytes followed by `@` (122 bytes total) |
| `CmdStartPage` | `1B 42 00` |
| `CmdEndJob` | `1B 45 1B 40 1B 5A` |
| `PaperSize/DHLShippingLabel` (`OptionID=335`) | `1B 4C 08 36 1B 77 03 C0` |

**Source evidence:** previously authenticated original ZIP
`LeitzIconLabelSetup(4).zip` SHA-256
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`,
original `Drivers/LeitzIcon.gpd` SHA-256
`f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`;
see `analysis/windows-2018/RECONSTRUCTION_STATUS.md`. The earlier
`tests/reference/WINDOWS_2018_BINARY_FINDINGS.md` file refers to a
**different installer extraction** and reports 120 ESC and 76 paper
options. Its values must not silently replace the newer verified
121-ESC/77-option inventory from `LeitzIconLabelSetup(4).zip`.

`gpd_commands.[ch]` uses a separate, bounded offline output API;
unknown commands and insufficient buffers are rejected without
modifying the caller output. `gpd_commands_test.c` exercises complete
byte-for-byte expected sequences and output integrity under GCC/Clang.
`sh test.sh` runs the tests; optional sanitizer runs can be added locally.

The **77 original paper options** (76 fixed, one `CUSTOMSIZE`) are
not reproduced from this one example. Do not interpret `DHLShippingLabel`
as a material cassette, mechanical capability or a mapping to any other
Linux/macOS format. The host-side **timing/order of GPD expansion relative
to the OEM `LeitzRp` plug-in** has not been reconstructed, nor have the
specific paper-selection logic and resolution commands. No device writes,
Win32 DLL execution, CUPS/USB integration or firmware changes occur.
