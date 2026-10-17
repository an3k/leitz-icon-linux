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


## Current extension: entire original Windows GPD media catalog

The earlier paragraph stating that only four GPD byte sequences were
represented describes the former fragment-only scope. The new
`gpd_media_catalog.[ch]` + generated `gpd_media_catalog_entries.inc`
extends that scope to **all 77 OEM GPD media selections**, while the
unchanged `gpd_commands.[ch]` still owns the document/page-level commands.

Original source records are preserved under
`analysis/windows-2018/windows_2018_original_media_inventory.json`,
with their independently parsed GPD text transcription in
`LeitzIcon_2018_GPD_text_transcription.gpd.txt`. The latter is *not*
a byte-identical raw OEM archive file. Every OptionID, rcNameID,
page/printable geometry, optional CUSTOMSIZE bound and CmdSelect byte
sequence is transferred directly; no geometry heuristic regenerates
the OEM command. The exact-generation script verifies correspondence
to both saved representations before checking the committed C table.

The original has **20 continuous selections** with ESC L FFFF and
**two length exceptions**: AppointmentCard and LeitzAppointmentCard
specify ESC L 023A even though their page height is 1200 (not 1140).
All Windows vendor UI strings must still be resolved independently
from LeitzUSRes.dll; numeric resource IDs are not display strings.

`GPD/test.sh` compiles and runs the added catalog under GCC/Clang.
CI also checks all 77 original records, all 76 fixed-geometry records
against a pinned development catalog and reproduces the complete
87-ID four-source comparison. This is an offline read-only data/command
reconstruction; no OEM Unidrv/COM ABI or hardware-printing parity.


## Static Windows GPD job sequencing (October 2026)

The prior bounded `gpd_commands` and complete 77-option
`gpd_media_catalog` modules already existed and remain **unchanged**.
The new `gpd_job_sequence.[ch]` composes these into the **static
GPD-declared** order rather than duplicating the manufacturer media
selection commands:

- `DOC_SETUP.1`: `CmdStartDoc`, 121 ESC bytes then `@`.
- `PAGE_SETUP.1`: `CmdStartPage`, `ESC B 00`.
- `PAGE_SETUP.2`: the exact original selected `PaperSize` `CmdSelect`
  from the 77-entry catalog; `CUSTOMSIZE` is its own 4-byte command.
- `PAGE_SETUP.3`: original resolution `Option1` = 300 × 300 dpi /
  `ESC h` (default), `Option2` = 300 × 600 dpi / `ESC i`.
- `PAGE_FINISH.1`: `CmdEndPage` explicitly contains zero bytes.
- `JOB_FINISH.1`: `CmdEndJob`, `ESC E ESC @ ESC Z`.

The original `Halftone` feature has four GPD options and no literal
`CmdSelect` in those blocks; no unknown OEM dither semantics or commands
are invented here. Input validation checks the entire page selection
before output. Unlike unknown OEM failure semantics, the portable
callback adapter stops after any nonzero callback result.

Regression `gpd_job_sequence_test.c` covers a two-page static sequence
(DHL 300×600 then CUSTOMSIZE 300×300), byte offsets, callback boundaries,
empty page finish, invalid selections and callback failures. Local
GCC/Clang (-std=c11 -Wall -Wextra -Werror -pedantic) and GCC ASan/UBSan
passed. CI compiles the new adapter against the **real already existing**
catalog and command modules. This does **not** prove actual Windows
Unidrv/OEM plug-in execution order, write-call framing or firmware
behavior and is not a CUPS filter.
