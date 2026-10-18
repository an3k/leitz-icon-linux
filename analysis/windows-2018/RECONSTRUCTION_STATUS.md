# Leitz Icon Windows 2018: Rekonstruktion begonnen

## Quellen und Schutz vor Doppelarbeit

Vor dem Commit geprüft: `sources=f46d5cbf730b0cf6746198c81e3a042950ce3ad8`,
`driver=3289f1bd644b19618b84a68931c8dd78a74e54ad`. Bereits
vorhandene Befunde `tests/reference/WINDOWS_2018_BINARY_FINDINGS.md`
wurden herangezogen, nicht als vollständig vorausgesetzt.

Originaldateien des Nutzers (statisch gelesen, niemals ausgeführt):

- `LeitzIconLabelSetup(4).zip`: SHA-256
  `cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`,
  8.273 ZIP-Einträge, darunter vollständige x86/x64-Treiberdateien und
  `Software/PrintHelper.dll`.
- `Leitz-Icon-Windows-Release(1).exe`: SHA-256
  `7ccd9299cb25b0dddcb5dbf414d7f905cab3029fb97c4dc7e6bb840e8b096452`.
  WiX-Burn-Bootstrapper mit PE-Overlay; sein eingebettetes Payload wurde
  **noch nicht** mit den Dateien aus dem ZIP gleichgesetzt.
- `Drivers/OEMPRINT.INF`: `DriverVer=03/15/2018,2.17.4.112`;
  `Drivers/LeitzIcon.gpd`: SHA-256
  `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`.
- Original `Drivers/amd64/LeitzRp.dll`: SHA-256
  `876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787`.
  Seine PE32+-Instruktionen (ImageBase `0x180000000`) wurden mit
  `objdump -d -M intel` offline disassembliert.

## Wichtige Berichtigung: 77 Windows-Medieneinträge

In **dieser tatsächlichen Original-GPD** stehen **76 feste Formate plus
`CUSTOMSIZE` = 77 Einträge**. Die bisherige Inventur im Branch `tests`
nannte nur 75 feste und damit 76 insgesamt. Ursache: `DHLShippingLabel`
bei GPD-Zeile 1036 beginnt mit TAB-Einrückung und wurde beim Zählen
von nur vier Leerzeichen eingerückten `*Option`-Zeilen ausgelassen.

Der originale zusätzliche Eintrag:
`DHLShippingLabel`, `OptionID=335`,
`PageDimensions=(1039,4204)`, `PrintableArea=(960,4144)`,
`PrintableOrigin=(40,30)`; die GPD-Sequenz ist
`1B 4C 08 36 1B 77 03 C0`.

Die komplette 77er-Inventur wurde mit einer klammerbewussten,
einrückungsunabhängigen Auslese des Nutzer-ZIP erstellt und separat als
JSON-Ergebnis ausgegeben. Das ist **nicht** dieselbe Anzahl wie die 86
Linux-Entwicklungsprofile; Aliase und Kassetten sind gesondert zu
beurteilen. Die vier Medienquellen Linux/macOS/Windows/Eigenentwicklung
bleiben für den genauen Quervergleich getrennt. Die originalen
Windows-Formatnamen dürfen nicht stillschweigend geändert werden.
`CUSTOMSIZE` hat keine feste Papierhöhe und darf keine globale
170-mm-Längenbeschränkung erhalten.

Die Original-GPD enthält außerdem exakt **121 ESC-Bytes gefolgt von
`@`** im `CmdStartDoc` (122 Bytes) sowie `CmdStartPage=ESC B 00`,
`CmdEndJob=ESC E ESC @ ESC Z`, 300×300- und 300×600-Auflösung,
vier Unidrv-Halbtonoptionen und medienabhängige `ESC L` / `ESC w`.
GPD-Optionen sind nicht automatisch zusätzliche Hardwarefähigkeiten.

## Direkt rekonstruierter Windows-Rasterpfad

Relative Adressen innerhalb der **originalen** x64 `LeitzRp.dll`:

- `0x26df–0x2720`: 32-Bit-Breite
  `raw_length-leading_blank_bytes` vergleichen mit dem
  nullerweiterten 16-Bit-Cached-Wert an internem Offset `+0x08`.
  Bei Unterschied 3 Bytes `ESC D <width&255>`, erst nach einem
  Resultat ungleich `0x80004005` den Cache auf 16 Bit aktualisieren.
- `0x2725–0x274b`: **immer** separat `ESC B <margin&255>`;
  `0x80004005` ist ein Abbruchcode.
- `0x2907–0x29a7`: komprimierte Länge **echt kleiner** als Rohdatenlänge
  -> `0x17` plus kodierter Puffer, sonst `0x16` plus Rohdatenpuffer.
  Selector und Inhalt werden in getrennten WriteData-Aufrufen ausgegeben.
- `0x2774–0x27cc`: in einem bestimmten Fall maskiert das Original die
  unbenutzten Bits des letzten Rasterbytes **im Eingabepuffer**.
  Dieser potenziell schädliche Seiteneffekt wird nicht blind übernommen.
- `0x2060–0x237a`: weiterer Windows-Rasterpfad mit `ESC w`,
  Leerzeilensprung `ESC f` (16-Bit-Länge), Etikettenabschluss
  `ESC G` sowie Seiten-/Etikettenzähler. Vollständige Bedingungen
  und die Sicherheitsgrenzen sind **noch nicht** portiert.
- `0x1600–0x1ed7`: aus Unidrv ermittelte Papiernamen
  (`Continuous`, `CUSTOMSIZE`), `ModeAuto`, `IsDieCut`,
  Etikettenlänge. Vollständige Zustandsmaschine bleibt offen.

Die getrennten C-Dateien unter `reconstructions/windows-2018/LeitzRp`
stellen lediglich die belegten ersten drei Pfade unter einer **neuen**
portablen Test-API nach. Original-COM-ABI, Windows-Encoder,
Ausnahmefälle sowie direkter Laufzeitvergleich mit der Windows-DLL
sind noch nicht nachgewiesen. Die eigenständigen Tests mit GCC und
Clang sind erfolgreich; **kein physischer Drucktest**.

Die originale Diagnose `Job size is multiple to 64` ist **kein**
Nachweis einer allgemeinen Padding-Pflicht: kein automatisches
Padding oder geänderter Druckabschluss.

## Nächste Arbeitsschritte

1. Kompressionsroutine und ESC-f-/ESC-G-Zustandsübergänge gegen beide
   DLL-Architekturen vollständig rekonstruieren; Roh- und komprimierte
   Ausgaben als Golden Vectors vergleichen.
2. Windows `ModeAuto`, `IsDieCut`, `iLabelLength`,
   `LabelsCount`, `CUSTOMSIZE` und GPD-/DevMode-Verknüpfungen
   implementieren und mit bestehenden CUPS-Medien verknüpfen.
3. Originale Status- und bidirektionale Protokolle über
   `LeitzLm.dll`, `PrintHelper.dll` prüfen; keine unbelegten
   Gerätebefehle senden.
4. Alle vier Formatinventare nebeneinander zuordnen und
   integrierte Hardwaretests durchführen.

Prioritäten bleiben: **druckfähig > Linux-Originalfunktionen >
alle Herstellerfeatures kombiniert > Standardkonformität >
veröffentlichungsreif > Golden Sample**.

**Keine Herstellerbinaries im Git. Keine Änderung an `driver`,
`main`, `cups`. Keine Firmware- oder Geräteoperationen.**


## OEM row-state machine from Windows LeitzRp x64 RVA 0x2060–0x237a

**Verified existing work first:** sources HEAD 0691484d3455800e4babc7eb8ec30c492cbb6ae8.
The earlier filter_graphics_fragments.c already reconstructs ESC D,
ESC B and the 0x16/0x17 payload selector; none of these were rewritten.

**Manufacturer evidence:** User-provided ZIP cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca,
Drivers/amd64/LeitzRp.dll SHA-256
876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787.
Static Intel x64 disassembly, DLL ImageBase 0x180000000.
Native function 0x180002060–0x18000237a:
- 0x20f2–0x212a: when state+0x0e <= 1 and DWORD state+0x18 is
  nonzero, send ESC w + two-byte big-endian WORD state+0x1c;
  the OEM does not inspect this WriteData HRESULT.
- 0x215a–0x2187: if medium+0x10 WORD is nonzero, label
  count WORD state+0x0a is not 1, and WORD state+0x0e is 1,
  send ESC E, again not checking its HRESULT.
- 0x218c–0x21e5: scan leading 0x00 bytes.
- 0x21eb–0x2258: if the row is nonwhite and pending blank
  counter state+0x10 is nonzero, issue ESC f followed by a
  two-byte big-endian pending count, clear the counter BEFORE
  checking for precisely 0x80004005 (E_FAIL), and abort only on
  that HRESULT.
- 0x225d–0x226d: call the separate row/filter routine at RVA
  0x26a0 with original bytes, total length and leading-zero
  offset. The caller does NOT inspect that routine's HRESULT.
- 0x22b3: on all-white or zero-length row, increment the
  16-bit queued blank counter; if row index == label-height
  WORD at +0x12, flush its ESC f.
- 0x2272–0x22ae and 0x231e–0x2352: at exact equality
  row index == target height, reset DWORD at state+0x0e
  (both row index and pending count), set previous-width
  state+0x08 to 0xffff BEFORE sending two-byte ESC G, then
  increment label counter +0x0a; ESC G HRESULT ignored.

**Implementation:** reconstructed row_control.[ch] is a separate
portable C module implementing defined state transitions with an
independent row encoder callback; exact internal C++ type and COM
linkage are NOT claimed. It avoids unsafe original zero-length
reads. The caller owns the row counter; original automatic
counter updates and complete Windows compression are not claimed.
Existing raster command helpers, protocol modules, media profiles
and unrelated branches are unchanged.

**Offline verification:** GCC and Clang (-std=c11 -Wall -Wextra
-Werror -pedantic -O2) passed isolated C unit tests covering:
queued blank-row count, nonblank flush, blank-page ESC f/ESC G,
width and medium prefix commands, pending counter reset timing,
E_FAIL abort before pixels, other HRESULT ignored, callback
error ignored at final row, and state reset before ESC G.
The tests are static manufacturer-instruction-derived vectors,
NOT executable dynamic parity against the original DLL or actual
hardware. The sources Windows CI workflow runs the same checks.

**Still open:** complete Windows compressor, caller-owned
row-counter / ModeAuto / IsDieCut / CUSTOMSIZE logic, exact
COM ABI, bidirectional status framing, and printer hardware.
Only sources is modified.


## Reconstructed Windows x64/i386 native RLE bit codec

**No duplicate work:** confirmed existing sources HEAD
4c1524d199332fa0b068a675e0c1904be97633af and prior passing Windows
CI run 38882692199. The previous ESC D/B raw-vs-compressed framing
helpers and ESC w/E/f/G counted-row controller remain untouched.

**Original evidence:** From the user-provided Windows 2018
Drivers/amd64/LeitzRp.dll (SHA-256
876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787)
and Drivers/i386/LeitzRp.dll (SHA-256
7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33),
disassembled without executing either binary:

- x64 RVA 0x274c–0x2905 and x86 RVA 0x2800–0x2910 accumulate
  contiguous identical raster bits, MSB first. The previous run
  starts with a special sentinel 0xff. Each emitted run is
  1..128 bits; low seven encoded bits are run length minus one,
  high bit is run color (1=black, 0=white).
- x64 RVA 0x27fe–0x283b and x86 RVA 0x2836–0x288b implement
  a shortcut for full 0x00 or 0xff bytes while the 128-bit
  limit remains satisfied; otherwise bits are counted individually.
- x64 RVA 0x2907–0x2925 / x86 RVA 0x2910–0x2944 select
  compressed bytes only when strictly shorter than raw bytes,
  otherwise raw selector 0x16. The existing, independently
  reconstructed fragment-selection/WriteData code is not modified.
- x64 RVA 0x2774–0x27cc conditionally clears unused low bits
  of the final raster byte when the source row's byte length
  exactly equals ceil(physical bit width / 8). The original
  mutates its own source buffer. The portable C helper instead
  copies the row and masks only the copy.

**Scope and design:** rle_windows_2018.h/.c reconstructs the
defined compressed-byte codec with an entirely new portable C
API. The function takes already-trimmed bytes, requires
adequate caller-provided output storage, stops when compression
cannot beat raw length and allocates no memory. The optional
clipping helper rejects physical width zero and out-of-range
inputs, rather than imitating original undefined memory
behaviors. It does not claim the original private COM ABI or
unsafe side effects.

**Tests:** GCC/Clang strict warnings with a separate bitwise
run reference, exhaustive deterministic lengths 0..96 for 25
patterns per length, explicit runs crossing 128 bits,
compressed/nonbeneficial selection, malformed pointer/capacity,
and final-byte mask cases. This is offline
*instruction-derived source behavioral verification*, not a
direct Windows-native DLL oracle, not a live printer test,
and not complete label-length/ModeAuto functionality.

**Still open:** native Windows private COM semantics,
caller-maintained variable-length/die-cut/copy modes, unusual
buffer and Windows exception paths, original execution parity,
and hardware validation. Productive driver, main and cups
branches remain unchanged.


### Separate assembled command-stream tests

After the native x64/i386-derived RLE implementation, a separate
test assembles the existing width/margin fragment functions with the
new compressor and the original selector. Original-instruction-derived
expected byte streams:
- Two leading white bytes + four black bytes:
  \`1b 44 04 1b 42 02 17 9f\`, four WriteData chunks (3/3/1/1).
- The next line has identical dimensions but alternating bit content:
  \`1b 42 02 16 aa 55 aa 55\`, three chunks (3/1/4);
  repeated ESC D is suppressed.
- 33 valid pixels with seven unused low bits cleared on a COPY:
  \`1b 44 05 1b 42 00 17 a0 06\`, four chunks.

The standalone helper returns USE_RAW with encoded-length zero when
compression does not improve. The existing selector's contract is
a *candidate length comparison*; the integrated caller therefore
passes the raw-data candidate and its actual length in that case,
rather than manufacturing an empty encoded candidate. The original
fragment module is left untouched. These are offline deterministic
tests, not claims of native Windows DLL execution parity.


## Windows 2018 LeitzRp initial media/label length states (x64 RVA 0x16f7–0x1c4e)

**Work-state check:** inspected actual sources HEAD
52923a334170eba0b8b8dfb2e8058f148524fc1f
and previously successful Windows CI 38885136374. The complete
row-count/ESC f/G and RLE implementations already exist and have
not been rewritten.

**Original manufacturer facts:** Offline disassembly of the
SHA-256-authenticated LeitzRp.dll 2.17.4.112 x64 shows:

- 0x16fa: native state DWORD at offset +0x08 initialized
  to 0x0001ffff (16-bit previous width=0xffff; label counter=1);
  0x1701 clears DWORD +0x0e (current row and queued blank rows);
  0x1793 writes QWORD 1 to native state +0x14
  (IsDieCut=1, IsCustomSize=0 initially).
- 0x18b6–0x18ed uses case-sensitive wide-character substring
  searches: "Continuous" => IsDieCut DWORD +0x14 becomes 0,
  "CUSTOMSIZE" => IsCustomSize DWORD +0x18 becomes 1.
  This is not an exact-name equality test.
- 0x19a2–0x19d4 calls GDI GetDeviceCaps(10=VERTRES,
  8=HORZRES). OEM controls are read from source pointer +0x30:
  WORD +0x12 is ModeAuto; WORD +0x0e is iLabelLength.
- 0x1b33–0x1b77: only when (IsDieCut OR IsCustomSize)
  and DEVMODE WORD +0x4c (dmOrientation) == 2, output
  width WORD +0x1c is the VERTRES low 16 bits and paper
  height WORD +0x0c the HORZRES low 16 bits. Otherwise,
  0x1bb7–0x1bd7 sets these fields in the normal order.
- 0x1b7a–0x1bab / 0x1c12–0x1c3b: when ModeAuto==0,
  IsDieCut==0 and IsCustomSize==0, label-length WORD
  +0x12 is taken directly from the OEM iLabelLength WORD
  +0x0e. Otherwise label-length is the chosen page
  height after any landscape swap. This is conditional,
  NOT a global fixed-label length.
- OEM debug strings at original RVA 0x46b0, 0x4a60,
  0x49e0, 0x4b00 and 0x4b80 corroborate the names
  IsDieCut, IsCustomSize, ModeAuto and iLabelLength.

**Source implementation:** label_setup.h/.c adds a portable,
separate media-state initializer connected to existing
leitz_win2018_row_control fields. The existing field
send_width_flag is now documented to map to original
IsCustomSize at native +0x18 (row OEM 0x20f9, ESC w).
The external OEM medium +0x10 flag is intentionally preserved,
not invented or reset. Source API accepts already-resolved
GetDeviceCaps raster dimensions, input name and ModeAuto/
iLabelLength options: it neither calls Windows APIs nor
replaces the Linux CUPS media catalog. Error/null handling
is deliberately safer than unspecified manufacturer behavior.

**Regression cases:** standard, custom-size, fixed die-cut and
continuous media; landscape rotation versus intentionally
non-rotating continuous material; ModeAuto off/on; explicit
job-dependent iLabelLength; case-sensitive substring;
native 16-bit dimension truncation; state reset and external
flag preservation; invalid pointers. Tests run under GCC
and Clang in the existing Windows-only CI workflow.

**Boundary:** No native Windows DLL execution parity, Windows
private COM structure equivalence, mechanical hardware tests
or productive driver modifications are asserted. The original
Windows option names remain untouched. No 170-mm universal
restriction or guessed media-ID conversions introduced.


### Media setup -> row output end-to-end regression

In a separate regression linking `label_setup.c` with the already
reconstructed `row_control.c`, a CUSTOMSIZE page emits
`1b 77 00 05 1b 66 00 01 1b 47` (ESC w / ESC f / ESC G) over
three independent native-shaped callbacks; the existing encoder callback
is a counted stub. With non-diecut continuous stock and ModeAuto=0,
the exact specified `iLabelLength=4` yields
`1b 66 00 04 1b 47`, notwithstanding raster-height=2. Setting
ModeAuto=1 produces `1b 66 00 02 1b 47` instead. No guessed 170mm
cap, no row-counter hidden increments, no direct manufacturer DLL
execution parity. The existing modules are unchanged.


## Windows LeitzLm 2018 language monitor dispatch and bounded response collection

Sources branch inspected at 0a79146fbe15f7236a3ab09a39d2a53d5fc92ac0
before execution; LeitzRp raster, RLE, ModeAuto and ESC f/G work
already complete and untouched. New static OEM evidence from
SHA-256 8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d
(\`Drivers/amd64/LeitzLm.dll\`; PE32+, ImageBase 0x180000000):

- RVA 0x100d..0x1094: exact UTF-16 \`lstrcmpW\` names at
  RVAs 0x34d0, 0x34f8, 0x3520. Returns FALSE for unrecognized
  requests with no worker call; supported calls send exactly
  two bytes ESC A, ESC U, ESC V respectively, then store
  the numerical worker result (reported reply size) and
  return TRUE independently of the worker's size.
- RVA 0x1750..0x1851: common helper calls underlying write
  callback, creates the read thread, waits 5000 ms and
  forcibly terminates it. This dangerous Win32 behavior
  was NOT copied into our Linux-facing code.
- RVA 0x1860..0x1aee: ReadPort worker initializes a 1024-byte
  aggregate, requests 1023 bytes, sums returned read sizes,
  and stops if accumulated count equals the expected size
  or the final byte is NUL. The native worker uses full-size
  read requests even after some bytes accumulate: unsafe
  exceptional overflow behavior is intentionally not copied.

\`reconstructions/windows-2018/LeitzLm/status_dispatch.[ch]\`
reconstructs the three documented command bytes and the
bounded collection predicate behind a new portable callback
interface. Dedicated GCC/Clang tests cover recognized and
unrecognized requests, exact casing, worker failures,
nonblocking segments, exact-size and NUL termination, and
buffer-bounds checks. No USB I/O, firmware modifications,
status parsing or native Windows DLL dynamic parity.
Response framing per command remains unestablished.


## Windows PrintHelper 2018 native DEVMODE editor: SetPaper and SetCustomPaper

Duplicate-work check: sources HEAD 023e7dda705dc7a4c38f7e1e6eca2a142d30c382
and most recent Windows CI 38888936221 success. LeitzRp RLE,
ModeAuto and LeitzLm dispatch/response parsing already recovered;
none rewritten in this phase.

Original binary: PrintHelper.dll i386 SHA-256
0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a.
Disassembled PE32 image base 0x10000000, 9 exported functions
(including CanReadFromPrinter, GetPaperDefs, IsPrinterOnline,
RetrieveStatus, SendRecvData, SetPaper, SetCustomPaper,
SetPrinterWirelessNetwork and ReflashPrinter).
Only these two entry points were implemented here:

- \`SetCustomPaper\` RVA 0x1930..0x1b24: open Windows printer,
  fetch DEVMODEW via DocumentPropertiesW, replace DWORD
  \`dmFields\` at +0x48 with 0x2402, patch DEVMODEW WORD at
  +0x4e (dmPaperSize), +0x60 (dmYResolution), +0x5a
  (dmPrintQuality). Original 0x1a6d..0x1a8d computes end
  pointer using 16-bit fields dmSize (+0x44) and
  dmDriverExtra (+0x46), then patches OEM-private WORDs
  at end-0x0a, end-0x0e and end-0x10 respectively from
  exported function parameters at +0x18,+0x1c,+0x20.
- \`SetPaper\` RVA 0x1b30..0x1d2d: same Windows DEVMODE
  preparation, but original 0x1c43 first checks the existing
  DM_PAPERSIZE bit (0x02) in dmFields and takes a failure
  path if absent. It then sets dmFields=0x2402, the same
  three public WORD fields and OEM-private end-0x0a from
  parameter +0x18, end-0x10 from parameter +0x1c.
  Original 0x1c90 writes the constant WORD 1 at end-0x0c
  and does not modify end-0x0e in this branch.
- Both write back via DocumentPropertiesW and SetPrinterW
  level 9. These are *spooler-side settings*, not
  additional ESC/device transport commands.

The independent portable \`reconstructions/windows-2018/PrintHelper/\`
module performs these strictly bounded DEVMODEW patches only.
Its public signature is NOT the original Windows DLL ABI.
The OEM does not perform equivalent bounds validation; our code
rejects dmSize < 0x62, private-extra length < 16, and buffers
too short for the advertised size before any write.
Tests verify public offsets, original-private negative offsets,
explicit DM_PAPERSIZE gate, preserved untouched bytes,
little-endian data, nulls and malformed length descriptors.
GCC and Clang offline passed before commit. A separate
Windows CI job runs it beside the already passing raster and
language-monitor tests.

**Outstanding:** Original Windows spooler/COM linkage and
all exception paths, response framing/status semantics,
GetPaperDefs paper-capability mapping, further PrintHelper
functions, original DLL dynamic parity, and real hardware tests.
Do not substitute this Windows-only DEVMODE manipulation for
the existing Linux CUPS custom-length and media logic.

## PrintHelper GDI paper geometry and spooler-stored data

Work-state check: sources HEAD 172d83bb0b2a5431dca734c134fc142d4b7f1409,
last complete Windows CI 38899970271 successful. Existing
LeitzRp, LeitzLm and PrintHelper DEVMODE implementations remain
unchanged.

Original i386 DLL: PrintHelper.dll, SHA-256
0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a,
PE ImageBase 0x10000000. Imported GDI32 CreateDCW, GetDeviceCaps,
DeleteDC and WINSPOOL.DRV SetPrinterW/GetPrinterDataW directly establish
that these two exports act on Windows printer properties, NOT on
the Leitz Icon USB protocol.

GetPaperDefs original RVA 0x1210..0x1500: argument +0x0c points to paper IDs in 4-byte slots
(the OEM loads the low WORD at base + index*4 at RVA 0x13d3),
+0x10 is the entry count; output array
pointer arguments at +0x14/+0x18/+0x1c/+0x20/+0x24/+0x28 each
receive count DWORDs. At 0x13c3..0x13e6 the code sets
PRINTER_INFO_9.DEVMODEW.dmFields = 2 and dmPaperSize from each
caller-supplied paper ID, then calls SetPrinterW level 9. It creates
a DC at 0x13ee..0x1400 and calls GetDeviceCaps six times at
0x1402..0x1464 in the exact order
112 PHYSICALOFFSETX, 113 PHYSICALOFFSETY, 110 PHYSICALWIDTH,
111 PHYSICALHEIGHT, 8 HORZRES, 10 VERTRES. It then destroys the
DC. These are physical/printable **page and raster properties**,
not an automatic cassette or label-type lookup. The original
loop does not explicitly restore the previous selected paper.
No new hardware command is established by this export.

RetrieveStatus original RVA 0x15f0..0x1715 invokes GetPrinterDataW
at 0x169f for the **caller-provided Windows printer data value
name**, using a 128-byte local buffer. It obtains pcbNeeded via
the last argument, then allocates based on that length and copies
pcbNeeded bytes from the local buffer at 0x16e0..0x16f2, without
checking that the result is within 128 bytes or that the API
returned success. This is a safety defect in exceptional cases,
not valid evidence for a larger device response or a USB status
protocol. Exact stored value names and meanings remain open.

Added separate portable C implementations in
reconstructions/windows-2018/PrintHelper:
- paper_geometry.[ch]: callback-driven, side-effect-free GDI geometry
  retrieval for supplied paper IDs, preserving the six independent
  values. Does not call SetPrinterW, Windows GDI or any device.
- printer_data.[ch]: safe, bounded retrieval analogue for a named
  spooler property. Error and over-capacity responses do not modify
  destination buffers. Uses no Windows API, USB or CUPS I/O.

Offline GCC/Clang tests exercise exact query order, distinct
physical-versus-printable dimensions, selection/query failures,
partial-record protection, 128-byte limits and unchanged output
on failed named-value retrieval. These are source-derived mocked
tests, not original DLL execution parity. No originals were
modified or committed. The active driver, main, cups and tests
branches remain unchanged.

Outstanding: SendRecvData COM protocol and response framing, direct
OEM DLL runtime comparisons, real hardware tests, complete four-source
media inventory mapping and true cassette capabilities.


## Queue work-offline attribute and direct ESC V version probe

Duplicate-work check: before modifying sources, verified the actual
sources HEAD \`047b450c218fbcd5b8b50dde4da9e4a55696c312\`, the
previous Windows CI 38902630112 (success) and existing
PrintHelper DEVMODE, GetPaperDefs and RetrieveStatus source
implementations. None of these components was overwritten.

Original user-provided PrintHelper.dll PE32 i386 SHA-256
\`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`:

- \`IsPrinterOnline\` at 0x10001510..0x100015ef opens a spooler
  queue, obtains \`PRINTER_INFO_2\` via two GetPrinterW(level 2)
  calls, then checks \`[record+0x34] & 0x400\` at 0x100015a2.
  On 32-bit Windows this is
  \`PRINTER_INFO_2.Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE\`.
  It does NOT inspect \`PRINTER_INFO_2.Status\` at +0x48 and does
  NOT communicate with the physical USB/network printer.
  Calling its outcome hardware online status would be wrong.
- \`CanReadFromPrinter\` at 0x10001110..0x1000114a calls the
  internal routine at 0x10002270 and returns true precisely
  if that internal return value equals 1. The helper writes
  the two bytes \`1b 56\` (ESC V) at 0x100022b3, requests a
  64-byte reply at 0x100022d5..0x100022e7, and searches that
  reply for ASCII \`BOOT\` at 0x100022f1..0x10002304. It sets
  a separate boot flag only if the substring is present;
  otherwise it leaves that flag unchanged. The helper
  returns 1 at 0x10002324 on successful transfer
  regardless of whether BOOT matched, and returns 101 at
  0x1000234c after an unsuccessful transfer. This is a
  version/boot-mode query, NOT a complete status parser.
  The original uses \`strstr\` on a 64-byte fixed response and
  could overread if the data is not NUL-terminated.
- Distinct from the earlier \`LeitzLm\` status command selectors
  ESC A, ESC U and ESC V. The identical ESC V command does not
  make the Windows spooler work-offline attribute a hardware
  response. The actual returned version layout and BOOT
  meaning beyond the case-sensitive substring are still unknown.

New isolated portable \`status_probe.[ch]\` checks only these
defined boolean decisions, with bounded NUL-aware substring
scanning. GCC and Clang unit tests validate queue attributes,
success independent of BOOT, case sensitivity, preservation of
preexisting BOOT state and malformed response bounds. No
Windows calls, USB writes, firmware changes, native DLL execution
parity, direct hardware tests or driver integration.

\`SendRecvData\` remains incompletely reconstructed: native RVA
0x1720..0x191d initializes COM and creates two different
CoCreateInstance objects using UUID constants at RVAs 0x443c,
0x43ec, 0x441c and 0x43cc; it sets the UTF-16 string
\`ReadWriteUse\`, then uses COM vtable dispatch and conditionally
allocates/copies a response. Without authenticated interface
definitions or runtime traces we must not invent protocol frames
or claim its private COM methods are known. Firmware and Wi-Fi
exports are not exercised.


## SendRecvData native COM call order and safe positive-result transfer

Native \`PrintHelper.dll\` i386 RVA 0x1720–0x191D was analyzed
offline from the supplied manufacturer's binary (SHA-256
\`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`).
No executable was launched. The original Export \`SendRecvData\`
accepts at least five 32-bit stack parameters at offsets
+0x08, +0x0c, +0x10, +0x14 and +0x18 (the last two are output
pointer destinations, assigned only in the nonempty result branch).
Source-level meanings of private COM methods are NOT presumed.

Confirmed CoCreateInstance pairs, using the native Win32 GUID
byte order (all GUIDs decoded from actual RVAs):
- Class at 0x443c: 2a614240-a4c5-4c33-bd87-1bc709331639
  with interface ID at 0x43ec:
  d580dc0e-de39-4649-baa8-bf0b85a03a97.
- Class at 0x441c: b9162a23-45f9-47cc-80f5-fe0fe9b9e1a2
  with interface ID at 0x43cc:
  8f348bd7-4b47-4755-8a9d-0f422df3dc89.

Original instructions and distinct vtable offsets:
1. 0x1762–0x179f: CoInitialize, CoInitializeEx, create
   first COM object, invoke vtable+0x0c with original first
   argument and literal 2. HRESULTs of object creation
   are not adequately guarded against invalid pointers.
2. 0x179f–0x17d7: create second COM object; invoke its
   vtable+0x0c using the exact UTF-16 text
   \`ReadWriteUserData\` at original RVA 0x41cc,
   **not** the formerly truncated \`ReadWriteUse\`.
   Invoke second object's vtable+0x10 with the literal 7
   and the original input buffer/length parameters.
3. 0x17d7–0x1822: invoke first object's vtable+0x14
   with the UTF-16 literal \`Get\` at original RVA 0x41f0.
   Invoke second object's vtable+0x14 and check BOTH its
   HRESULT and a separately returned zero DWORD at stack
   -0x18. Only when both are zero invoke the second
   object's vtable+0x18. The latter's result must equal
   zero to follow the positive-data branch.
4. 0x187c–0x18bb: only when the returned byte count at
   -0x2c is nonzero, allocate that number of bytes with
   HeapAlloc(GetProcessHeap,0,n), set the caller's
   output pointer and output length, and copy the COM
   buffer at -0x28 using memcpy. Original does not
   adequately verify HeapAlloc's result or the private
   COM-returned size; potential invalid reads/writes
   are NOT portable features to reproduce.
5. 0x18bb–0x18ec: free the COM-allocated source with
   CoTaskMemFree, release two COM objects and uninitialize
   COM. Native boolean return is true only if the COM
   returned a nonzero byte count; a zero-length or failed
   operation returns false. The Windows heap result has
   a distinct ownership and cannot be treated as borrowed
   memory.

\`sendrecv_result.[ch]\` is a NEW portable, bounded model of
**only** the positive-data result/length decision. It tests
all three required zero status/result fields, requires
nonzero reply length, transfers bytes only into an explicitly
bounded caller-owned destination, and leaves destination and
published length unchanged on failure. It never calls COM,
never allocates an unverified Windows heap block, and does
not claim the original DLL's ABI or wire protocol. Unit tests
exercise independent COM failure gates, positive binary data,
empty reply, invalid pointers/capacity and overlapping buffers.

No registration/IDL files identifying these private
interface methods were found in the uploaded ZIP. The
GUIDs and invocation slots are therefore **proven**,
whereas method names, internal call semantics and the
actual USB payload framing remain **unknown**. No device
command, firmware or Wi-Fi operation is performed.

Verified pre-write sources HEAD: 2eb5b69690033a23c336e41fc9e46703da144b0c. Previous Windows CI 38908617680 completed successfully. Earlier Win32-spooler, media, LeitzLm and raster reconstructions were not changed.


## Windows PrintHelper SetPrinterWirelessNetwork: ANSI payload transfer

**Duplicate-work check:** sources HEAD
38ca610ae4b165c5763190e97f949a9d061dff72, prior passing
Windows CI 38911116602. No prior SetPrinterWirelessNetwork source
implementation was present. All existing LeitzRp, LeitzLm and
PrintHelper files/behaviors are preserved.

**Original manufacturer instructions:** \`PrintHelper.dll\` from
original uploaded ZIP, SHA-256
\`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`,
PE32 i386 ImageBase 0x10000000. Export \`SetPrinterWirelessNetwork\`
RVA 0x1150–0x120a uses arguments +0x08 (device-path string) and
+0x0c (configuration string). At RVA 0x1157 it calls helper 0x2140,
which wraps \`CreateFileW\` with \`GENERIC_READ|GENERIC_WRITE\`
for the supplied device path. At RVA 0x116b it creates an internal
UTF-16/BSTR-like holder for the configuration, then when needed
at RVA 0x1183 invokes helper 0x2ad0 to convert to bytes.
The helper uses \`WideCharToMultiByte(CP_ACP=0, flags=0)\` with
the UTF-16 input string length INCLUDING its trailing NUL;
the ANSI code page is selected by the Windows host and not a
fixed encoding supplied by the manufacturer.

At RVA 0x1194–0x11b1, the original computes \`strlen\` on the
converted byte string and passes that length (EXCLUDING NUL)
and the byte pointer directly to the common \`WriteFile\` wrapper
at 0x1e20. No ESC or proprietary prefix is assembled within
this export. The bytes are **opaque configuration data** supplied
by the caller; the content structure and firmware processing
remain unknown. A direct write to a chosen device path is NOT
the previously reconstructed \`SendRecvData\` private COM path.
The writer uses an overlapped event and may block on
\`ERROR_IO_PENDING\` via \`WaitForSingleObject(INFINITE)\`,
then calls \`GetOverlappedResult\`. There are native error
and null-pointer hazards; \`CloseHandle\` occurs after attempting
the write. No new USB command frames can be inferred from this.

**New portable bounded reconstruction:** the
\`wireless_payload.[ch]\` model accepts a bounded UTF-16 string,
a platform-provided ANSI conversion callback, and a synthetic
writer callback. It preserves the manufacturer's
\`WideCharToMultiByte\` then \`strlen\` boundary, including
truncation on converted embedded NUL and excluding the trailing
NUL from transmission. Unlike the OEM, missing terminators,
invalid conversion length and null pointers are rejected.
Temporary bytes are zeroized after conversion/delivery.
Non-ASCII handling is delegated to a codepage-aware callback,
not guessed to be UTF-8 or Latin-1.

**Proof boundary:** GCC and Clang strict compilation and
offline tests cover ASCII, a synthetic non-ASCII mapping,
early NUL, zero-byte payload, writer error, conversion error,
missing termination, oversize reports, buffer/pointer errors
and temporary-byte zeroization. These tests prove the new
portable wrapper's behavior against instruction-derived cases,
NOT full native Windows DLL execution parity, the unknown
configuration syntax, actual WLAN association or printer access.
No user credentials, network communication, firmware flashing
or actual device commands occur in tests.

**Separate firmware feature:** \`ReflashPrinter\` is the native
firmware-changing export at RVA 0x29a0, which must not be called
as part of discovery, print status or WLAN setup. Firmware
flashing remains expressly excluded from offline reconstruction
execution; it requires independent verification and physical
recovery provisions before integration.


## Windows application LabelSizes.xml versus Windows GPD (distinct OEM inventories)

Checked sources HEAD 4902026bd1a4c6ebe452857c50e6bb7ba7fa3495
and Windows CI run 38916773516 (success) before continuing.
The Windows GPD parsing, x86/x64 reconstruction and all
existing tests were retained; no earlier code was replaced.

A second ORIGINAL Windows paper-format source occurs in the same
user-provided 2018 ZIP:
\`LeitzIconLabelSetup/SourceDir/Leitz Icon Software/LabelSizes.xml\`,
11,827 bytes, SHA-256
\`3e0ce0029cb864daa1100b2e15816ec66705e2246b26fe6af14707350e9bf2e1\`.
This is separate from
\`Drivers/LeitzIcon.gpd\` (SHA-256
\`f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7\`).
No binaries were executed.

Exact inventories for these particular manufacturer files:
- Windows application XML: **37 unique label sizes**, including
  **10 continuous** entries (no fixed height).
- Original Windows GPD: **77 options** = 76 fixed plus CUSTOMSIZE.
- Exact matching internal identifiers: **36**.
- Present only in the application XML: **Continuous112**.
  It is already present in the Linux-development media catalog,
  sourced there to Original Linux 1.0.3 / macOS 1.2.190 PPD.
  Its absence from the Windows GPD is NOT evidence that the
  hardware lacks that cartridge/size.
- Present only in the GPD: **41** options, including the
  Leitz-prefixed variants, rotation variants, Wristband,
  and CUSTOMSIZE. These are not silently discarded.
- Of the 27 shared FIXED (non-continuous) XML entries,
  **9** have direct width/height agreement with GPD and
  **18** have transposed width/height agreement, after
  applying exactly the declared GPD \`*MasterUnits: PAIR(300,600)\`
  and a 1 mm tolerance for catalog rounding.
  **0** fixed shared entries have a larger difference.
- Among the original metric versus imperial UI strings,
  \`NameBadgeLabel70060001\` is a distinct manufacturer
  inconsistency: metric values 76×50 mm but inch fields
  2×3 inches (equivalent to 50.8×76.2 mm), i.e. swapped.
  Do NOT change original labels or swap axes globally.

The Windows application dimensions describe catalog labels,
the GPD dimensions are printer page/raster geometry; neither is
automatically the cartridge's physical feed width, the CUPS media
dimensions, nor the printable imageable area. GPD PrintableArea
and PrintableOrigin are retained as separate original values.
The 64000/65535-style GPD lengths for continuous media must
not be treated as an absolute hardware length limit.

Added \`analysis/windows-2018/inventory_windows_label_sizes.py\`
which REUSES the already-committed GPD reader rather than
implementing a second GPD parser. It inventories original XML
and GPD with SHA-256 provenance, retains literal manufacturer
IDs, original metric/imperial strings, GPD OptionID and
page/printable/origin values, and distinguishes orientation.
The parser rejects invalid dimensions/duplicates and
unrecognized printer MasterUnits instead of guessing.
\`test_inventory_windows_label_sizes.py\` is a standalone
Python unittest suite with synthetic fixtures, and CI runs
it without distributing any OEM installer or media file.
A full original-file inventory was executed locally and
its counts verified before committing the parser.

This comparison does not replace the pending full four-way
inventory of Linux OEM, macOS OEM, Windows OEM and Linux
development; it exposes an additional INTERNAL Windows
source that had been missing. No changes to driver/main/cups.


## Original Windows 2018 AddPrinterTool HTTP WLAN control-panel protocol

**State check:** inspected HEAD
\`f6f8040953885d228539bb4f2eb02b40f93e8a50\`, with the prior
Windows-only application LabelSizes.xml/GPD comparison and passing
CI run 38920033343 already present. No earlier driver, parser,
Wi-Fi payload, status, COM or media work was repeated.

**New manufacturer PRIMARY evidence:** user-provided Windows
ZIP \`LeitzIconLabelSetup(4).zip\` contains managed
\`LeitzIconAddPrinterTool.exe\` (522,752 bytes, SHA-256
\`e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435\`).
The .NET CLR metadata #~ MethodDef table, #US user string
heap and method IL \`ldstr 0x70xxxxxx\` tokens were parsed
offline without executing the installer. The following
strings were located INSIDE the specific live method bodies,
not inferred merely from the file's unassociated strings:

- \`GetBaseUri\` MethodDef 263, RVA 0x62bb: \`http://\`.
- \`GetWiFiScanInfo\` MethodDef 267, RVA 0x6548:
  \`Scan.xml?scan=1\`, \`Scan.xml\`, \`ScanInProgress\`,
  \`NumScanResults\`, \`MySsid\`. Indicates scan initiation
  and response attributes; actual response examples unavailable.
- \`GetWiFiNetworkInfo\` MethodDef 269, RVA 0x67e4:
  \`ScanRow.xml?ScanRow=\`, \`response/bss\`, \`Valid\`,
  \`Name\`, \`SecurityMode\`, \`NetworkType\`, \`Strength\`.
  Names are genuine XML element/attribute lookups in the OEM
  application. XML structure cannot be certified from bare
  text constants alone.
- \`SetWiFiNetwork\` MethodDef 270, RVA 0x6960:
  \`Main.html\`, \`POST\`, \`application/x-www-form-urlencoded\`,
  literal \`&\` and \`%26\`, plus the EXACT form template:
  \`HiddenName=&HiddenSecurity=None&HiddenPassword=&DHCP=DHCP&WifiSetup=Connect&NetworkType=Infrastructure&Name={0}&SecurityMode={1}&SecurityKey={2}\`.
  This is a separate network-control-panel mechanism from
  PrintHelper's direct-device \`SetPrinterWirelessNetwork\`.
  The method uses three form substitutions (network name,
  security mode, key); the full argument-specific escaping
  behavior is not yet proven. Do not assert full parity
  for arbitrary strings, characters or Windows codepages.
- The same assembly also contains \`WEP\`, \`WEP40\`,
  \`WEP104\`, \`WPA\`, \`ScanInProgress\`, \`MySsid\`,
  \`WlanConnect\` and a separate SoftAP workflow, confirming
  the Windows installer supports more than a blind USB write.
  No general claim of printer firmware support for every
  wireless security mode or implicit credential validity.

**Offline C code:** \`reconstructions/windows-2018/WiFiHTTP/\`
contains \`wifi_http.c/h\` and tests. The two scan URIs
are reconstructed exactly (relative paths, not hardcoded
device addresses). The infrastructure POST builder emits
the complete proven OEM field names/order and uses a
bounded, standards-compliant percent-encoder for supplied
UTF-8 byte strings. It does NOT claim that the OEM's
original code percent-encodes every special character the
same way; byte parity is limited to unreserved ASCII
arguments. The module contains no HTTP or device access,
checks capacities, rejects embedded NULs, and supplies a
scratch zeroization helper because the result contains
a sensitive Wi-Fi key.

**Verification:** strict GCC and Clang C tests cover exact
manufacturer scan paths, maximal decimal scan index,
literal form-body parity for unreserved ASCII, safe encoding
for delimiter-containing inputs, short output capacity,
invalid buffers and result zeroization. Tests are offline
and synthetic: no OEM .NET code executed, no actual
manufacturer HTTP response captured, no WLAN configuration
sent, no firmware operations performed. Complete OEM
HTTP error semantics, authentication, actual XML samples
and private COM method meanings remain open.

**Next work:** crosscheck the actual HTTP response fields
and authentication/session handling with ORIGINAL managed
IL, then integrate supported controls behind opt-in
authenticated transport and physical device tests, without
disturbing the working Linux CUPS raster printer.


## Recovered AddPrinter managed scan retry, security modes and original ASCII form

**No duplicate work:** sources HEAD 4c88c09d489d7ec73b37ff6302cd0c60e64ce4a3
verified before writes. The existing \`wifi_http.[ch]\` remains
standards-oriented; this **separate** \`wifi_managed.[ch]\` captures the
distinct original Microsoft .NET application's behavior, not
a modification of the already passing implementation.

Primary binary: Windows 2018 \`LeitzIconAddPrinterTool.exe\`
SHA-256 e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435.
Read PE/CLI metadata streams #~, #Strings, #US and MethodDef IL
**offline**, without executing the application or making HTTP requests.

- \`GetWiFiScanInfo\` MethodDef 267, RVA 0x6548, IL SHA-256
  308b5b972e83576255c219e763de99534102adc1127c108a97060c9ba33066a7:
  first requests \`Scan.xml?scan=1\`, parses first response node;
  if \`ScanInProgress\` is not \`"1"\` (case-sensitive String.Equals,
  StringComparison.Ordinal=3), reads \`NumScanResults\` and \`MySsid\`.
  Otherwise polls \`Scan.xml\` immediately and then with 300ms sleeps
  while in progress, limited to 50 follow-up retrievals. Missing or
  empty response yields null, and caught exceptions are logged.
  No invented unlimited polling or synchronous background tasks.
- \`GetWiFiNetworkInfo\` MethodDef 269, RVA 0x67e4, IL SHA-256
  ebb1583d45a94d6fe1330f9a0f5e11d2d48995f867ad4daab4409ec0ae255993:
  selects nodes via \`response/bss\`, inspects the first child
  node's \`Valid\`, \`Name\`, \`SecurityMode\`, \`NetworkType\`
  and \`Strength\` with SelectSingleNode and get_InnerText;
  compares Valid with "1" using StringComparison.Ordinal.
  The latter three numeric values are parsed from XML text
  by the application's managed Integer.Parse method.
  No actual hardware XML response was present in the upload;
  only selector identities and control flow are verified.
- \`GetSecurityModeString\` MethodDef 271, RVA 0x6a88, 112 IL bytes:
  mode 0 => None, mode 1 => WEP, with password length 5/10 -> WEP40,
  13/26 -> WEP104; modes 5,9,13 -> WPA; other modes -> None.
  These are original application numeric **enum values**, not
  invented firmware capability guarantees.
- \`SetWiFiNetwork\` MethodDef 270, RVA 0x6960, IL SHA-256
  9921566b368276b37b6432256ba75c3980f0512a85d5ea103b092cdd86df035c:
  fetches \`Main.html\`, sets credentials, constructs the literal
  form with String.Format, replaces \`&\` with \`%26\` on
  **only the security key**, not on SSID or mode. It obtains
  \`System.Text.Encoding.Default\`, converts the formatted body
  into bytes and sends POST with \`application/x-www-form-urlencoded\`.
  There is **no evidence** the OEM percent-escapes every input value.
  Earlier generic RFC 3986 encoder therefore must not be mistaken
  for original byte parity. It also uses credentials inherited from
  the WebRequest, which have not been independently reconstructed.

\`wifi_managed.[ch]\` includes:
1. Exact IL-derived numeric mode/key-length mapping.
2. OEM-compatible ASCII form builder (raw SSID/mode; only
   ampersands in key replaced by %26). Non-ASCII is explicitly
   excluded because Windows \`Encoding.Default\` depends on its
   configured legacy codepage.
3. Pure finite scan progress state with an initial fetch and
   at most 50 follow-ups; delay and actual HTTP remain outside
   this offline C API. A final 300ms sleep before OEM time-out
   is described, not automatically executed.

Strict GCC and Clang tests cover modes, WEP lengths, byte-for-byte
ASCII form including unescaped SSID '&', insufficient buffers,
non-ASCII rejection, first scan, polling exhaustion, and immediate
completion. No network configuration, device requests, personal
Wi-Fi credentials, firmware changes or productive Linux code.


## OEM AddPrinter managed scan and BSS XML response selection

**Pre-write check:** sources HEAD a11679f61568ac948dfe6ead514f482d6bdbb0a8.
Previous managed GetWiFiScanInfo retry (50 attempts, 300 ms),
GetWiFiNetworkInfo selectors, original security mapping,
ASCII form-encoding implementation and all existing tests already
present. No existing source routines were overwritten.

**Primary evidence:** user-supplied original Windows 2018
LeitzIconAddPrinterTool.exe, SHA-256
e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435.
.NET IL MethodDef 267 RVA 0x6548 uses the literal child names
ScanInProgress, NumScanResults, MySsid. MethodDef 269 RVA 0x67e4
selects response/bss, its first child, then literal child
names Valid, Name, SecurityMode, NetworkType, Strength.
The original compares progress/valid with the **exact** string
"1" (StringComparison.Ordinal), and parses the three latter
BSS numeric texts as signed Int32 values. Actual OEM responses,
XML schemas, namespace conventions and missing-node semantics
are not available in the user upload.

**New offline code:** wifi_xml.[ch] models only those SELECTED
XML fields in portable C using Expat. The fixture envelope
\`<response>...\` and row wrapper \`<response><bss><row>...\`
are **inferred** from the managed selector paths; no claim of
actual response-schema parity. XML is decoded with entity
handling, rejects DTD, does not fetch network resources,
bounds input to 64 KiB, individual field text to 255 bytes,
and rejects repeated known fields, invalid integers,
malformed XML and over-nesting. It treats in-progress scans
as having no result count and invalid BSS rows as having no
detail values. All original source-code modifications are
standalone in sources, not the production Linux driver.

**Offline verification:** GCC and Clang -std=c11 -Wall -Wextra
-Werror -pedantic -O2, with local AddressSanitizer and
UndefinedBehaviorSanitizer runs, passed. Synthetic cases
include XML entity decoding, exact "1" versus whitespace-
padded text, signed fields, missing/invalid/overflow integer
fields, DTD injection, repeated field, truncated document,
unknown fields and unchanged caller output after errors.
The GitHub Windows-only workflow separately compiles/runs
the same C tests with system Expat. No real HTTP request,
printer reply capture, credentials, COM calls, USB writes
or firmware operations were performed.

**Still open:** OEM-authenticated HTTP response examples,
exact XML root/wrapping/error semantics, retrieval and
authentication handling, true OEM application execution
comparison and physical device tests. Original managed scan
and security-mode policies were not duplicated or modified.


## Original Windows AddPrinterTool WiFi password validation patterns

Pre-write verification: branch sources HEAD
70d6203592e101b436cfd0de40a2781d27b91d89;
the prior Windows managed scan/HTTP form, XML replies and raster
reconstructions already exist and are left unchanged.

Primary source: the user-provided Windows 2018 installer
LeitzIconAddPrinterTool.exe (SHA-256
e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435).
Its managed MethodDef 285 CheckWiFiPasswordPattern, RVA 0x6c34,
contains direct calls to security-code getter (MethodDef 292)
and Regex.IsMatch. Its static initializer MethodDef 286,
RVA 0x6d18, builds **five** Regex instances with exact literals:
- ^[0-9a-fA-F]{10}$
- ^.{5}$
- ^[0-9a-fA-F]{26}$
- ^.{13}$
- ^.{8,63}$

IL branch reconstruction:
- mode 0: returns TRUE before checking password;
- other modes: null/empty returns FALSE and resource key
  AddPrinterToolEnterPassword;
- mode == 1: accepts ANY of first four patterns above,
  otherwise resource key AddPrinterToolPasswordIncorrect;
- mode > 1 (signed comparison): requires final ^.{8,63}$
  match, otherwise incorrect-password resource;
- a negative numeric mode with nonempty password returns TRUE.
This differs from the separately reconstructed method
GetSecurityModeString (MethodDef 271): that mapping calls only
5, 9, 13 "WPA", while password validation applies 8..63
to *all* mode codes > 1. Never silently combine those decisions.

New portable, standalone C wifi_password.c/.h and test use
UTF-16 code-unit input to match .NET code-unit counts and support
the original Regex '$' before trailing LF, with explicit
input bounds. They do not store, log, send or modify the secret.
New error return for unterminated input is a safety extension.
No claim of identical .NET regex exceptions, case-folding for
other scripts, UI localization, printer security support,
managed-runtime parity or hardware authentication.

Locally tested with GCC and Clang -std=c11 -Wall -Wextra
-Werror -pedantic -O2, AddressSanitizer and
UndefinedBehaviorSanitizer; CI runs the same deterministic
functional cases without sanitizer dependencies.


## Windows AddPrinter and LeitzIconCommon HTTP credential decisions

**Preflight:** verified sources HEAD
45c83b26738e5408c533eb334626aff6b9bded32,
Windows CI 38940378698 successful and all previous WLAN scan,
XML, form and password validation files already committed.
This phase does not modify those files or production Linux branches.

**Primary OEM evidence:** original user-uploaded Windows 2018
LeitzIconAddPrinterTool.exe SHA-256
e27663457368f529cc64322a7550431ec2ff6b28e4d640a87419b837df924435,
and the independently shipped LeitzIconCommon.dll SHA-256
1cc54e495fae83dae22ea94559a6963d4a3d22f4abcb883550a81f7902758c00.
Decoded ECMA-335 MethodDef table, IL bodies and #US/#Strings streams
from each actual binary offline, without launching Windows/.NET code.

**Evidence by exact method RVA:**
- AddPrinter MethodDef 263 GetBaseUri RVA 0x62bb concatenates
  literal "http://" and the \`_host\` string, constructing Uri.
  This is cleartext HTTP, not authenticated TLS.
- AddPrinter MethodDef 264 IsPrinterControlPanelAvailable RVA 0x62d4
  creates Uri from GetBaseUri + literal "Main.html", then invokes
  the external WebSiteIsAvailable with numeric authentication
  flag 1 and the exact \`_user\`/\`_pwd\` fields; its catch returns 2.
- **Located and decoded the actual called implementation** in
  LeitzIconCommon.dll MethodDef 183 WebSiteIsAvailable RVA 0x3820:
  WebRequest.Create, cast HttpWebRequest, set 15000ms timeout,
  add new NetworkCredential(username,password) if flag true,
  set method GET, call GetResponse. Successful response returns 3.
  The WebException catch checks WebExceptionStatus == 7
  (ProtocolError), then HttpWebResponse.StatusCode == 401:
  in that exact case return 1. All other WebExceptions and
  general exceptions yield 2 (name-resolution failure has an
  explicit log branch, but same numeric return). Response is
  disposed in the finally path.
- AddPrinter MethodDef 460 IsConnectedToNetwork RVA 0x8334
  calls GetCurrentSSID, rejects null/empty current SSID,
  then performs an ordinal case-sensitive String.Equals
  against its supplied argument.
- AddPrinter MethodDef 280 IsConnectedToSameWiFi RVA 0x6bab
  passes its \`_host\` property to IsConnectedToNetwork.
  The same value also supplies GetBaseUri; we do NOT assume
  host and wireless SSID are always semantically identical.
- AddPrinter MethodDef 281 CheckLogin RVA 0x6bb8 first
  calls IsConnectedToSameWiFi: false => immediate status 2;
  true => return the actual IsPrinterControlPanelAvailable status.
- AddPrinter MethodDef 279 ProcessCredentials RVA 0x6b60 calls
  CheckLogin, immediately returns if status==3, else performs a
  **second** IsConnectedToSameWiFi observation. If the second
  check is false return 4; if true return the original status.
  The two connection checks are not logically interchangeable.
- AddPrinter MethodDefs 266 and 267 (GetPrinterConnectedWiFiName
  and GetWiFiScanInfo, RVAs 0x64b4 / 0x6548) create a separate
  web client with a **18000ms** timeout and set
  NetworkCredential(_user,_pwd) before DownloadString.
  This is distinct from LeitzLm's 5000ms status-reader timeout.

**Implementation:** new standalone portable \`wifi_auth.[ch]\`
models UTF-16 string comparison, WebException/HTTP status mapping
and both login/connection decisions with an independent C API.
It does not issue GET/POST, handle plaintext credentials,
configure Wi-Fi or change CUPS. No credentials are stored.
Invalid enum and pointer cases are rejected rather than copied
from unspecified OEM exceptional behavior.

**Tests:** strict GCC and Clang standalone tests cover
case-sensitive code-unit comparisons, missing SSID, HTTP 401
with ProtocolError, HTTP 403 / other exception values,
successful HTTP response, both network observations, status 3
short-circuit, and 15s/18s timeouts. Locally passed offline.
Native .NET execution parity, real HTTP transport and hardware
validation remain outstanding.


## Windows AddPrinter complete credentials decision ordering (offline)

Pre-write check: `sources` HEAD
`780e8abb1cd596fd5a21055ac5d61be2952de801` before this
new work, with `wifi_auth.[ch]` and its tests already committed.
Their existing code was not modified.

Original evidence: Windows 2018 AddPrinter MethodDefs
279 (`ProcessCredentials`, RVA 0x6b60), 281 (`CheckLogin`,
RVA 0x6bb8), 280 (`IsConnectedToSameWiFi`, RVA 0x6bab),
264 (`IsPrinterControlPanelAvailable`, RVA 0x62d4),
and LeitzIconCommon MethodDef 183 (`WebSiteIsAvailable`, RVA 0x3820).
Method instructions and numeric status mappings were documented
in the previous section and are not re-derived here.

New `reconstructions/windows-2018/WiFiHTTP/wifi_auth_flow.[ch]`
composes the previously isolated observations and statuses in the
original observable order. It distinguishes the first and second
wireless observation; it skips panel access if the first fails;
it omits the second observation only for status 3; and it reports
status 4 for a subsequent wireless mismatch. The companion
`wifi_auth_flow_test.c` checks call ordering and error/HTTP cases
with synthetic observations. New CI step builds/runs it with
GCC and Clang. Local GCC/Clang `-std=c11 -Wall -Wextra
-Werror -pedantic -O2` checks, AddressSanitizer and
UndefinedBehaviorSanitizer all passed.

No original Windows application executed; no actual HTTP transport,
credentials, network association, COM call, USB operation or firmware
write was attempted. The portable callback boundaries are new,
not original Windows ABI. Complete Windows DLL reconstruction,
OEM execution comparison and physical tests remain open.


## Windows LeitzRp combined multi-label raster stream (offline)

Existing `label_setup`, `row_control`, `filter_graphics_fragments` and
`rle_windows_2018` functions were confirmed already present on sources
HEAD `dec4052a41c662ec048ac3909cc686be2b368413` and were
not changed. A new isolated `raster_stream.[ch]` adapter links these
reconstructed OEM-derived fragments under the same counted-row C API,
without adding guessed protocol bytes or any CUPS/USB code.

Synthetic stream regression checks two successive CUSTOMSIZE labels
(including conditional ESC w/E/f/G, ESC D/B widths and margins,
0x16 raw versus 0x17 RLE selection and 19 separate WriteData calls),
manual-length continuous material, unmodified source after final-bit
masking and the ignored FilterGraphics HRESULT versus diagnostic output.
Caller scratch storage is bounded and independently owned.
GCC, Clang, AddressSanitizer and UndefinedBehaviorSanitizer passed locally;
the Windows CI raster test script now runs the new case with both compilers.

Original 2018 Windows ZIP exists in the user's Library but its raw
bytes were not materializable in this session. Therefore there is no
fresh disassembly or dynamic vendor binary comparison, and the new
cross-module vectors prove only offline consistency with *previously*
recorded manufacturer-instruction-derived behavior. No production
branch, OEM source, firmware, credentials or hardware was modified.


## Windows PrintHelper SendRecvData: previously missing COM call sequence

Work-state check: `sources` HEAD
`a18e78c6575f5151ca8888ca18183a1539c2c41b`, with existing
`sendrecv_result.[ch]`, native GUID evidence and PrintHelper
tests present; none were replaced. Original PrintHelper.dll i386
SHA-256 `0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a`
was previously analyzed and the COM sequence documented at RVA
0x1720..0x191d (see above). Its raw bytes were unavailable to this
session; no fresh disassembly or Windows execution is claimed.

Added `sendrecv_sequence.[ch]`: callback-driven, standalone
C reconstruction of the *documented* first/second COM object
creation, vtable offsets, GUID pairs, literal arguments and two-result
status gate, using the already recovered bounded result transfer.
Previously unguarded pointer hazards intentionally take a safe path.
All acquired resources are cleaned on exit, including COM-owned
borrowed output. Not a native Windows COM implementation or new
USB protocol; calling semantics and native exceptional cleanup are
not claimed equivalent.

Offline regression: successful nonempty transfer, empty reply,
reported-status gate, nine independent callback-failure stages,
resource cleanup and overlong output rejection under GCC/Clang,
ASan/UBSan. Existing source modules and production branches
(`driver`, `main`, `cups`) untouched.


## Windows 2018 LeitzIcon.gpd fixed command-byte reconstruction (offline)

**Pre-execution state:** sources HEAD
`c1c23c356d60f696030aa2ad67def6c5319115f2` with all existing
LeitzRp, LeitzLm, PrintHelper, WiFiHTTP reconstructions present and the
prior Windows CI successful. None of those modules were changed.

Previously authenticated manufacturer input: original ZIP
`LeitzIconLabelSetup(4).zip` SHA-256
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`,
`Drivers/LeitzIcon.gpd` SHA-256
`f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`.
The earlier results documented these exact GPD bytes:
- `CmdStartDoc`: 121 consecutive ESC (0x1b) bytes and a
  terminal `@`, total 122 bytes;
- `CmdStartPage`: `1B 42 00`;
- `CmdEndJob`: `1B 45 1B 40 1B 5A`;
- `PaperSize/DHLShippingLabel` (`OptionID=335`):
  `1B 4C 08 36 1B 77 03 C0`.

New isolated `reconstructions/windows-2018/GPD/gpd_commands.[ch]`
provides a bounded callback-free copy of ONLY these byte vectors,
with a companion strict GCC/Clang test for every byte, truncated output
rejection and nonmutation on invalid requests. Local GCC/Clang,
AddressSanitizer and UndefinedBehaviorSanitizer tests passed. Windows
CI runs the separate `GPD/test.sh` script.

**Important source-conflict boundary:** the older
`tests/reference/WINDOWS_2018_BINARY_FINDINGS.md` from
a *different MSI extraction* still lists 120 ESCs and 76 media options.
That report is not silently conflated with the later GPD ZIP inventory
(121 ESCs; 76 fixed options and one CUSTOMSIZE, 77 total).
These are distinct recorded input/artifact findings, not proof
that the installers have identical GPD bytes.

The actual GPD host expansion and timing relative to the
LeitzRp COM/Unidrv filter, full 77-option Windows media mapping,
OEM runtime equality and physical printer operations remain unproven.
The new module does **not** send bytes to hardware or modify
the Linux production driver or any non-sources branches.


## Exact-name cross-source paper audit (partial Windows inventory)

Pre-write verification: sources HEAD f845265772ee53eb50727d4d9d86721e687748d1 and driver HEAD 3289f1bd644b19618b84a68931c8dd78a74e54ad. Previous C reconstructions, vendor files and all production branches are unchanged.

The new read-only PAPER_FORMATS_CROSS_SOURCE.md compares all 40 original Linux 1.0.3 PageSize IDs, 41 original macOS Classic PageSize IDs and 86 development media IDs using unchanged original readable labels. Exact matching internal names are used only for alignment, not proof of physical cassette equivalence. All 40 Linux IDs also appear in the macOS and development catalogs; macOS adds only DHLShippingLabel. The development catalog adds 45 IDs that are absent from both original PPDs.

The Windows 2018 GPD has 77 entries (76 fixed + CUSTOMSIZE) according to the previously authenticated ZIP; only two names are presently recoverable from the persisted findings: DHLShippingLabel with OptionID 335, and CUSTOMSIZE. The remaining 75 Windows names are expressly unknown, not omitted as presumed absent. The original archive bytes were not accessible in this session. Other Windows format equalities are not asserted.

Source discrepancies: Continuous70050001 is named "Continuous - 2 1/5\"" in Linux but "Continuous - 2 1/4\"" in macOS. The development media JSON incorrectly labels DHLShippingLabel as "not present in Windows GPD" despite original GPD evidence. No code/configuration correction of the development branch was made without separate verification.

Added test_paper_cross_sources.py, whose GitHub CI step checks every one of the 87 reported IDs (86 development + distinct Windows CUSTOMSIZE) against original Linux/macOS PPDs and the **pinned** development media catalog checked out read-only. It checks duplicate IDs, exact labels, subset relationships and known provenance contradictions. No new OEM commands or hardware capabilities are inferred.

## Vollständiger Vier-Quellen-Vergleich direkt aus verifizierter Windows-GPD (Werkzeug)

Prüfung vor Änderung: sources HEAD `8ce729d9a9b288cf26fb2e583a9985a11eab9f1d`, Windows-CI erfolgreich. Das Teilinventar `PAPER_FORMATS_CROSS_SOURCE.md` und das vorhandene Windows-GPD-Lesewerkzeug `inventory_windows_gpd.py` bleiben unverändert.

Neue Datei `compare_windows_gpd_four_sources.py` verwendet die bereits vorhandene originale GPD-Optionen-Auslese anstatt einen zweiten GPD-Parser zu schreiben. Sie akzeptiert das Herstellerarchiv und den separaten Entwicklungskatalog, verifiziert die Windows-GPD gegen den zuvor festgestellten SHA-256-Hash `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`, verlangt genau 76 feste Optionen plus `CUSTOMSIZE` und prüft das bekannte DHL-Profil gegen Original-ID, Geometrie und Befehlsbytes. Bei falschem oder fehlendem Original erfolgt ein Fehler statt einer gefälschten vollständigen Inventur.

Erstellt werden ausschließlich nach unmittelbarer Verifikation vollständige Vergleichszeilen aller vier Quellmengen: exakte interne IDs und originale Linux/macOS-Bezeichnungen, Windows-OptionID und rcNameID als nicht aufgelöste UI-Ressourcenkennung, originaler Windows-PaperSize-Befehl und der belegbare Geometrievergleich für wortgleiche IDs. Übereinstimmende Maße beweisen keine identische Kassette; fehlende Windows-UI-Klartextnamen werden nicht erfunden. `CUSTOMSIZE` bleibt ohne allgemeine Festlänge.

Neu: `test_compare_windows_gpd_four_sources.py` mit vier lokal bestandenen Tests (synthetische Optionen, fehlende Ressourcenbezeichnungen, Geometriedifferenzen, Duplikate, Original-Hashpflicht); in der Windows-CI enthalten. `FOUR_SOURCE_GPD_COMPARISON.md` dokumentiert den Aufruf. Das Original-ZIP ist im aktuellen Container nicht zugänglich; ein mit den echten 77 GPD-Einträgen erzeugter Bericht und ein Originalausführungstest werden daher ausdrücklich **nicht** als bereits erledigt gemeldet.

Produktiver Linux-Treiber, andere Branches, bestehende Quellen, Gerät und Firmware bleiben unangetastet.


## Windows-Original-GPD: alle 77 Namen nachträglich aus Projektbibliothek verifiziert

Vor Schreibvorgang geprüft: sources HEAD `1f16fcb597f59b18a16e1eb5506329458b0c4ae2`; letzte CI erfolgreich.
Die Library enthielt die Einzeldatei `LeitzIcon.gpd` und das früher direkt
aus demselben Original-ZIP gewonnene vollständige JSON `leitz_windows_2018_original_media_inventory.json`.
Alle **77 Original-GPD-Optionsblöcke** wurden in dieser Sitzung unabhängig
aus dem lesbaren GPD-Text extrahiert und mit allen 77 JSON-Datensätzen
nach Name, OptionID, rcNameID, Dimensionen, bedruckbarer Fläche,
Ursprung, Min/Max, originalem Cmd-Text und Befehlsbytes verglichen:
**77/77 identisch, null Feldabweichungen**. Der originale GPD-Rohbyte-SHA
wurde dabei nicht erneut berechnet, sondern nur der früher
gespeicherte Quell-SHA verglichen.

Der exakte Namensvergleich mit Entwicklungsbranch `3289f1bd` ergab
**76/76 feste Windows-Namen** im 86er-Entwicklungskatalog; alle drei
Geometriefelder pro gleichnamiger Option sind wertgleich. Die zehn
Entwicklungsbezeichner ohne Windows-Original-GPD-Eintrag wurden explizit
ermittelt und in `WINDOWS_2018_ALL_MEDIA_NAMES.md` konserviert; mit
Windows `CUSTOMSIZE` sind 87 eindeutige IDs in der vierfachen
Vereinigungsmenge. Der neue reine Offline-Test
`test_windows_gpd_names.py` prüft alle 87 Tabellenzeilen gegen
die beiden Original-PPDs und den gepinnten Entwicklungsbranch.

**Begrenzung:** Anschließend waren die rohen Bibliotheksdateien
nicht mehr lesbar/materialisierbar. Deshalb enthält der gesicherte
Namensbericht bewusst nicht die zuvor nur temporär gelesenen,
aber nicht vollständig in Git kopierten numerischen Windows-OptionIDs,
rcNameIDs und OEM-Befehlsbytes (mit Ausnahme des schon zuvor unabhängig
gesicherten DHL-Formats). Das bestehende Archiv-Hash-Prüfwerkzeug
`compare_windows_gpd_four_sources.py` bleibt unverändert.
Kein Format erhält einen erfundenen Originalbezeichner oder
einen aus der Eigenentwicklung abgeleiteten Windows-Befehl.

Keine Änderungen an anderen Branches, OEM-Dateien oder Druckhardware.


## All 77 Windows GPD media commands in original-derived C

Work-state checked: sources HEAD `9a8b9feffa5e6f6cc0406515011912e3f0a99a98`, Windows CI successful.
The original Linux 1.0.3 sources and macOS Classic original PPD/recovered
native code paths were reviewed alongside the archived Windows GPD.
The three manufacturer ZIP archives are listed in the project Library
but their raw bytes could not be mounted in this execution. Distinct
Linux, Windows and macOS print preambles/end-of-job semantics are
documented separately in ORIGINAL_PLATFORMS_COMPARISON.md.

Recovered the *entire* 77-record original Windows GPD paper inventory
and its independently verified normalized text transcription from
previously staged immutable Git blobs. The transcription is explicitly
not asserted byte-equivalent to the vendor original: the archived
Windows OEM GPD SHA-256 remains provenance only. Its 77 GPD option
blocks parse identically to all 77 preserved inventory records.

Added gpd_media_catalog.[ch] with 77 exact manufacturer per-media
command records in a generated .inc file. The Python generation
script verifies every preserved original GPD field before producing
the C initializer, and CI checks the committed initializer is
synchronized. Existing gpd_commands.[ch] continues to own the four
earlier independent command primitives; there is no replacement.

All 76 fixed Windows media IDs are found in the pinned development
catalog and match all three geometry arrays by exact ID, but the
original CmdSelect bytes are never recomputed from page dimensions.
Twenty continuous entries use length FFFF. Original nontrivial
exceptions AppointmentCard and LeitzAppointmentCard use length 570
despite 1200-unit page height; this is retained. CUSTOMSIZE has no
fixed height. No Windows resource-string names or physical cassette
mappings are invented.

Regression tests: all 77 GPD records vs inventory; 76 exact-geometry
matches; complete 87-ID four-source report; deterministic generated C
catalog; GCC/Clang bounded command copy, lookup, lengths, exceptions.
No real printer operation, COM/Unidrv execution or firmware changes.
Other branches and pre-existing reconstructed C remain unchanged.


## Windows 2018 static Unidrv GPD command ordering, isolated C adapter

Preflight checked `sources` HEAD `74ecc345c6d9646737712c2021645af7c0a2f600`: previous 77-entry
catalog, OEM-derived raster fragments and last successful Windows CI
already complete and not rewritten. Requested original archives
`LeitzIconDriversForLinux1.0.3.zip`,
`LeitzIconLabelSetup.zip`,
`LeitzIconSoftware-Installer_5.1.465.dmg.zip` were found in the
project Library but all three returned unauthorized/raw-path errors
when binary materialization was attempted. Reviewed instead the
original Linux 1.0.3 C filter, original 2018 GPD text transcription
and macOS Classic original PPD/native disassembly findings already
available on `sources`.

New `reconstructions/windows-2018/GPD/gpd_job_sequence.[ch]` is a
portable GPD-static-command **composer** using unchanged
`gpd_commands` and `gpd_media_catalog`. It preserves documented
`DOC_SETUP.1`, `PAGE_SETUP.1/.2/.3`, empty `PAGE_FINISH.1`
and `JOB_FINISH.1` GPD order, with actual manufacturer
`Resolution/Option1` (ESC h, 300×300 default) and `Option2`
(ESC i, 300×600) and the exact 77 OEM media CmdSelect choices.
`Halftone` GPD choices have no direct static GPD CmdSelect and
are deliberately not treated as recovered binary dither features.
Nothing imposes a fixed length for continuous stock.

Offline two-page byte-vector and write-call-boundary test,
invalid-option preflight and callback-error tests were passed under
GCC, Clang, AddressSanitizer and UndefinedBehaviorSanitizer locally
with explicit minimal stubs of pre-existing modules; GitHub CI
exercises the **actual** pre-existing modules. No OEM Windows executable,
private COM/Unidrv interface, physical printer, Linux production branch
or firmware was run. Do not conflate the static GPD command ordering
with an observed Windows runtime output stream; that integration remains
open.


## 2026-10-17: actual Windows OEM DevMode and paper UI DLLs finally inspected

Pre-work actual sources HEAD `27c8437c5d596b752244ea762abd12d3b6593886` and CI were checked. All three
explicitly uploaded original ZIP archives were opened and their SHA-256
values computed; full archive and individual original checksum evidence
is in `VERIFIED_UPLOADED_ORIGINAL_ARCHIVES.md`. Unlike previous runs,
raw binaries were **available** and were statically analyzed (never
executed).

Original `Drivers/i386/LeitzDevMode.dll` and
`Drivers/amd64/LeitzDevMode.dll` both export `hrOEMDevMode` at
RVA 0x1080. Native instruction analysis proved all four operations:
SIZE (writes 28 to OEMDMPARAM), DEFAULT (first 20 bytes of a 28-byte
OEM record only), CONVERT/MERGE (same helper at RVA 0x1000: compare
signatures at +4, copy min private lengths, force output size=28 and
field +8=1; mismatch logs "Unknown signature" and resets first
20 bytes). The new `LeitzDevMode/oem_devmode.[ch]` preserves these
decisions under a new bounded portable C API; x86/x64 OEMDMPARAM
layout and actual spooler callbacks remain unimplemented. Native
invalid-mode error 87/E_FAIL is preserved via optional callback.
Offline C tests passed GCC/Clang/ASan/UBSan.

The Windows original x86/x64 `LeitzUSRes.dll` binaries were read from
their **PE RT_STRING resource trees**, both giving identical 77 original
en-US strings with IDs 259..335. The 76 fixed PaperSize entries
point to exactly 76 of those names. DLL ID333 was deliberately kept
distinct from GPD CUSTOMSIZE's symbolic StdNames string. All original
UI spellings are archived in `windows_2018_original_ui_strings.json`;
`FOUR_SOURCE_WITH_WINDOWS_UI_NAMES.md` provides the complete
87-ID media comparison with original Windows readable names alongside
Linux, macOS and development IDs/names. An independent
`GPD/gpd_ui_resources` C lookup module reuses the preserved GPD
catalog and verifies all 76 mappings, while keeping numeric original
IDs separate from UI text. The unmodified manufacturer binaries are
NOT uploaded to GitHub.

Neither the original Windows DLLs nor actual printer communications
were executed. Production branches, preexisting C reconstructions
and source-original files are unchanged.


## Windows 2018 LeitzUI.dll: GetInfo and cut-option synchronization

Current-state check before work: `sources` HEAD
`ef3cfffca9a803aed797de5372e2fcb26d1d2c85`; latest
Windows regression completed successfully. Existing LeitzUI
reconstruction did not exist. Actual DLLs from the previously
reuploaded Windows manufacturer ZIP were used:
- x86 SHA-256
  `b6f066d87ac18aff5d37c6b4ac0c3c13d6e0cd58bb2afb5fb74152e3e99a5293`;
- x64 SHA-256
  `7d514bd5bb4c7610569c0aacd7b37cc74318e5fce858bf4fadba9bb554de0a10`.

Independent native disassembly establishes OEM UI GetInfo:
x86 RVA 0x1140 / x64 RVA 0x1150, returning signature DIPL for
mode 1, DWORD 1 for mode 3, and documented error codes 50, 87,
122 on unsupported/invalid/undersized requests.

The OEM PrinterUICallBack is at x86 RVA 0x1770 and x64 0x17d0;
the document UI callback is at x86 RVA 0x17a0 / x64 0x1820.
Both manipulate the private CutAfterEachLabel selection on callback
reason 6; the document UI also implements reason 10 (APPLYNOW)
and reason 11 (KILLACTIVE). Only a changed low-WORD selection
causes replacement of the private DWORD, OR 0x8 into its flags,
and callback return 1; KILLACTIVE restores the selection.
The native private value lives at +0x10 for x86 and +0x18 for
x64, not at a universal fixed offset.

Added separate portable `reconstructions/windows-2018/LeitzUI/`
C source with optional SetLastError callback. The normalized C
state is **not** the original COM/Unidrv struct and it deliberately
rejects invalid pointers in place of undefined native behavior.
Local GCC/Clang -Wall -Wextra -Werror -pedantic -std=c11 O2,
GCC AddressSanitizer and GCC UndefinedBehaviorSanitizer passed;
its four C/test/script blobs were checked against the exact
locally tested Git hashes. A separate Windows CI job step now
compiles and tests it alongside all previous work.

The archive files were previously SHA-256-inventoried and the
original Linux filter, macOS Classic filter native boundaries and
Windows cutter commands were reviewed. These UI callbacks do
not establish actual physical cutter timing. Original Windows
DLL execution, COM class factory and full UI integration remain
open, as do hardware print/cut regression tests. No production
branches or existing reconstructed code were replaced.


## LeitzUI.dll native COM factory, QueryInterface and unload gates

Duplicate-work check: sources HEAD
`3beeaa939aba275f945e18596f944c40e68c4005`, successful prior
CI, existing LeitzUI GetInfo/cutter callbacks retained unmodified.
Actual uploaded Linux, Windows and macOS ZIPs were hashed/inspected
again, and the original LeitzUI x86/x64 binaries were examined
directly with PE objdump, not executed.

New `reconstructions/windows-2018/LeitzUI/ui_com_factory.[ch]`
recovers the recorded x86/x64 DllGetClassObject, factory/UI
QueryInterface, IClassFactory::CreateInstance, LockServer and
DllCanUnloadNow return-code and GUID decisions. The byte-faithful
UUID set comes from the original .rdata of both DLL architectures:
CLSID abce80d7-9f46-11d1-882a-00c04fb961ec;
OEM UI IID c6a7a9d0-774c-11d1-947f-00a0c90640b8,
plus standard IUnknown and IClassFactory.

The native code checks the factory's CLSID, initializes the returned
interface pointer to NULL, refuses unknown interfaces, rejects
aggregation for CreateInstance, returns E_OUTOFMEMORY when native
allocation fails, and maintains a separate LockServer counter.
DllCanUnloadNow returns S_OK iff both observed native global
counts are zero. QueryInterface + Release follows factory and
instance allocation. Native DLL function RVAs and original error codes
are documented in the LeitzUI README.

The new C module is deliberately a bounded portable decision model:
it does not implement Win32 IClassFactory vtables, reference-managed
native allocations, OEM UI dialogs or COM registrations. Its result
represents which original interface would be selected, not a
native Windows object. Reference-counter lifetime and Linux CUPS
integration are still unverified. The previous GetInfo/checkbox
module was not changed.

GCC/Clang strict C11, ASan and UBSan regression passed locally
on independent GUID vectors, success and failure branches and
server-lock/unload paths. The existing LeitzUI GitHub CI script
was only extended to compile/test the new isolated component.
No production driver branch, original binary, actual device,
USB/firmware interface or existing regression was overwritten.


## OEM UI property-sheet data construction, x86 and x64

Before any change checked sources HEAD
`0255d6aa077febea7a5bfa4277beeb857d1c4e0d`
and successful CI. All three user-uploaded original manufacturer
archives were previously extracted and checked: Linux 1.0.3 original
raster2printer.c SHA-256
`bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0`,
Windows LeitzIcon.gpd SHA-256
`f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`,
macOS Classic original raster2printer SHA-256
`72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
The original Windows LeitzUI.dll i386 and amd64 were disassembled
directly. Existing UI callbacks and COM class factory were already
completed; they were preserved verbatim.

The next proven, previously unimplemented part is the
IPrintOemUI property-sheet **initialization** (document mode 1,
printer mode 2). x86 original constructor RVAs 0x1500 and 0x1630;
x64 0x14c0 and 0x1640. The new
`reconstructions/windows-2018/LeitzUI/ui_property_sheets.[ch]`
models the OEM item-count query, x86 0x34 / x64 0x58
OPTITEM layout, zeroing each item, first item flags=12 document
or 4 printer, zero-extended first selection from a source WORD,
one-option OPTTYPE/OPTPARAM allocations and fixed fields.
The source-specific native option item pointer, allocations,
Win32 text resources and callback **addresses** remain omitted:
the isolated model exposes normalized callback types and safe
zero pointer fields; no claim of complete Windows dialog parity.

Original X86 and X64 property-sheet functions were checked instruction
by instruction for all translated constants/offsets. Offline GCC,
Clang, GCC AddressSanitizer and UndefinedBehaviorSanitizer passed,
including callback/selection boundaries and deliberate safe rejection
of unsafe native short buffers. Existing LeitzUI CI script is extended
to run this third independent offline regression. Other branches
and reconstructed modules are not modified; hardware printing and
physical cutter operations remain unverified.


## 2026-10-17: original OEM UI driver-interface acquisition / release

Pre-write check: branch `sources` was
`8a4c9ba39b734788443deb98ec704c6fd02ce745`, last Windows
CI successful; LeitzUI/GetInfo, property sheets, GUID class factory
already existed and remain unchanged. The three original
user-supplied archives were actually opened and SHA-256-checked
again: Linux
`54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17`,
Windows
`cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`,
macOS
`6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210`.
The prior original Linux C, Windows GPD and macOS Mach-O
SHA-256 findings matched the newly opened ZIP contents.

From *actual x86/x64* OEM LeitzUI.dll disassembly, the previously
unimplemented method at RVA `0x1100` checks a cached
driver-interface pointer (x86 object +8, x64 object +16).
When NULL it calls supplied IUnknown's QueryInterface
(vtable first method) with actual OEM driver interface UUID
`92b05d50-78bc-11d1-9480-00a0c90640b8`
(x86 .rdata RVA `0x313c`,
x64 RVA `0x3210`), caches successful nonnegative
HRESULT output; maps negative HRESULT to E_FAIL and clears
cache. The original object's final Release path (x86
`0x10a0`, x64 `0x1080`)
invokes IUnknown::Release on a non-NULL cached interface.
This is not a derived or guessed USB printer protocol.

New isolated `ui_driver_interface.[ch]` uses safe
callback-based, caller-owned C state rather than opaque Win32
pointers. It does not replace existing COM class factory,
Win32 native ABI, reference counts, property sheets, UI
callbacks, printer traffic, or any production Linux driver.
Tests for both C compilers, AddressSanitizer and
UndefinedBehaviorSanitizer passed locally; precise source
blob SHA-1 values were compared to the locally tested C files
before commit. The existing LeitzUI/test.sh is amended
without re-running or rewriting earlier C implementation.


## Original LeitzRp raster-plugin COM host interface and factory

Before changes: `sources` HEAD
`f1b38b9f2b1d22aae0dcf73d556e16a249d37b86`;
existing LeitzRp row_control, label_setup, RLE, raster_stream,
LeitzUI callbacks/factory/driver binding and Windows CI were
confirmed present and not rewritten.

Original Linux 1.0.3, Windows 2018 ZIP and macOS Classic 5.1.465
were reopened and SHA-256 checked; original Windows i386
LeitzRp.dll `7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33`
and amd64 LeitzRp.dll
`876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787`
were directly inspected with objdump, no OEM execution.

New independent C modules under
`reconstructions/windows-2018/LeitzRp/oem_host.[ch]`
and `oem_factory.[ch]` capture previously absent,
directly disassembled original x86/x64 raster OEM GetInfo,
driver-interface QueryInterface cache/release, DllGetClassObject,
factory/raster QueryInterface, CreateInstance, LockServer and
DllCanUnloadNow decisions. Tests and specific RVAs are in
`reconstructions/windows-2018/LeitzRp/README.md`.

Crucial observed facts: GetInfo mode 4 publishes required=8 and
returns E_FAIL without SetLastError; other invalid modes set error
87, modes 1/3 publish DWORD DIPL/1 with error 122 when output
is too short. DLL raster interface IID
`d67ebbf0-78bf-11d1-9480-00a0c90640b8` differs
from the host interface IID
`d67ebbf1-78bf-11d1-9480-00a0c90640b8`.
The original LockServer global address is not either original
DllCanUnloadNow tested global address (both architectures), so
the reconstructed model does not falsely couple their counters.

The portable APIs intentionally receive allocation outcomes,
callbacks and global count values as inputs instead of claiming
the complete original Windows COM ABI or Unidrv lifecycle.
No native Windows OEM binary was executed, no printer data was
sent, and no source of the active Linux driver was modified.
Local strict GCC, Clang, ASan/UBSan tests passed. Existing
LeitzRp/test.sh is extended to run the new modules and GitHub
CI must confirm integration with existing source regressions.


## Windows 2018 original raster COM 26-slot VTable inventory

Preflight: sources HEAD 375df14e6499e704f25a3fa0454e1b102732ec10, latest Windows CI successful.
Three OEM original archives reopened and hashed, original Linux C
and macOS Classic raster2printer SHA256 verified. Windows i386
and amd64 LeitzRp.dll were parsed directly from original ZIP;
their PE VTable RVAs 0x509c and 0x5298 both contain exactly 26
methods. Direct original instruction bytes prove 12 E_NOTIMPL
stubs (slots 13,14,16–25), slot 5 direct 0, slot 9 direct 1 and
slot 11 literal FilterGraphics string comparison (0 for equal,
1 for unequal). No method name is inferred for unrecognized slots.
Not a statement about hardware capabilities.

New isolated oem_method_map C and regenerated constant initializer,
arch-specific inventory JSON and original-PE verifier. The verifier
was actually run against all three original ZIPs and passed x86,
x64, Linux and macOS checks. C tests passed GCC/Clang/ASan/UBSan.
The original verifier can be run with ZIP arguments outside CI;
CI checks self-consistency and runs all C regressions. Existing
OEM DLL reconstructs, driver/main/cups and printer are untouched.


## Original x86/x64 LeitzRp IUnknown reference lifecycle

Verified preflight: `sources` HEAD
`c012d5d0508955926c43425d451e329a82224305`,
previous CI success. All three original ZIP archives available
and SHA-256 confirmed. The original Windows i386/amd64
`LeitzRp.dll` disassemblies establish AddRef/Release
at x86 RVAs `0x1070/0x1290` and
x64 `0x1060/0x1290`. Native raster object reference-count
offset is x86 `+4`, x64 `+8`, initial count 1.
Native object count is incremented during successful
CreateInstance and decremented on final destruction:
global RVA x86 `0x643c`, x64 `0x76f0`.
The final release also calls `IUnknown::Release`
for any saved host interface at x86 `+8`, x64 `+16`.

This tracked raster-object count differs from both
`LockServer`'s global and the two `DllCanUnloadNow`
globals. The new separate portable `LeitzRp/oem_lifetime.[ch]`
models this OEM reference lifecycle with atomic counts and
safe input validation. It does NOT substitute the native
Windows COM heap/vtable ABI; no actual printer operation occurs.

Strict GCC, Clang and ASan/UBSan offline tests passed,
including original AddRef/Release return semantics, final
host callback exactly once, factory/raster counter separation,
invalid state and overflow/underflow. The original
`LeitzRp/test.sh` is extended; previous reconstructions
and all production branches remain unchanged.

## Original-Rasteraufruf Slot 15: IsDieCut trennt zwei OEM-Pfade

Vor Änderungen wurde `sources` HEAD `aaf3d295a294d09ae6924721dacbcd6b3d622b0b`
mit erfolgreicher letzter Windows-CI geprüft. Die drei
Originalarchive waren lokal verfügbar und die SHA-256-Prüfungen
der Hersteller-Linux-C-, Windows-GPD- und macOS-Mach-O-Dateien
wurden erfolgreich wiederholt. Aus den originalen Windows
`LeitzRp.dll`-Binärdateien beider Architekturen ist für
Slot 15 erstmals die bis dahin externe Zeilenzähler-Aktualisierung
und die `IsDieCut`-Verzweigung direkt nachgewiesen:
x86 `0x1FF0`: Inkrement WORD +0x0A, Test DWORD +0x10,
dann `0x2020` oder `0x2380`; x64 `0x2040`: Inkrement
WORD +0x0E, Test DWORD +0x14, dann `0x2060` oder `0x2380`.

Neues `LeitzRp/oem_row_dispatch.[ch]` ruft für gestanzte
Etiketten den existierenden, **unveränderten** Rasterstream
nach Zählerinkrement auf. Für `IsDieCut==0` (Endlosmaterial)
muss ein eigener, bisher nicht rekonstruierter
Routinen-Callback vorhanden sein: ohne diesen keine Ausgabe.
Damit wird der früher in Teiltests mögliche Aufruf desselben
Zeilencontrollers für kontinuierliches Material ausdrücklich
nicht als vollständige native Windows-Pfadgleichheit ausgegeben.
Das ursprüngliche `label_setup` liefert `is_die_cut`,
und der neue Integrationstest verbindet die bereits vorhandenen
realen OEM-Teilmodule erstmals hinter dem rekonstruierten
externen Dispatch. Keine feste Etikettenlänge eingeführt.

GCC/Clang und lokale Sanitizer-Tests für den neuen Dispatcher
bestanden. Der CI-Test kompiliert den Integrationstest gegen die
existierenden Raster- und RLE-Module. Ein echter
Windows-/Unidrv-/Drucker-Abgleich steht weiter aus.


## 2026-10-18: original continuous-media raster path 0x2380 recovered

Before changes, the `sources` branch was at
`3d4597b01c999a80ff5a78f535c1a3797047c86a` with successful CI. All three original uploaded ZIPs
were reopened; original Linux filter, Windows i386/x64 raster
DLLs and original macOS Classic raster filter SHA256 matched
the existing verified archive records.

The manufacturer has **two distinct** raster slot-15 branches:
die-cut at x64 RVA 0x2060 (already recovered) and continuous
at x86/x64 RVA 0x2380 (previously missing). The original 0x2380
routine was analyzed in both arch-native disassemblies.
New `LeitzRp/oem_continuous.[ch]` reconstructs its row state
transitions, 14-row conditional skip, original requested-label
limit, physical-page row reset, queued blank feeding, exact
E_FAIL-only handling, forwarded encoding result and deferred
ESC G one row after the configured label height. The original
ModeAuto-dependent ESC f flush and native 16-bit wrapping behavior
are preserved. No fixed print-length limit is introduced.

An independent alternate callback `oem_continuous_bridge.[ch]`
connects this to the pre-existing `oem_row_dispatch`.
A strictly additive `raster_stream` public wrapper exposes
the pre-existing private encoder without duplicating RLE or
modifying its implementation. Standalone GCC/Clang/ASan/UBSan
continuous-control tests passed offline; the new integrated
CI regression compiles against all the existing real
raster setup, prefix, RLE and stream modules, and checks a
19-row continuous print/cut command vector. As before, COM
and actual hardware parity remain pending. No production
branch is changed.


## Original Windows spool WriteData and raster VTable slot 8

Preflight confirmed sources HEAD `e3f98bf489e3eaea55a47091fbbf2b8874a89972` and successful CI.
All three original manufacturer archives re-opened; Linux
raster2printer.c, Windows LeitzIcon.gpd, macOS Classic raster2printer,
and x86/x64 LeitzRp.dll hashes matched prior inventory.
Original x86 RVA 0x29f0 and x64 RVA 0x29e0 were directly
disassembled: add requested bytes to a 32/64-bit accumulator
BEFORE invoking COM host write; only exact E_FAIL or a partial
write causes host error report 200 and overall E_FAIL.
Other failed HRESULTs with a full write are treated as success
by the original. The counter is not rolled back after failures.

Original VTable slot 8 x86 RVA 0x14e0 / x64 RVA 0x14f0
writes one literal ESC byte only when accumulated_bytes
== (5 * medium WORD+0x0c + 16), then frees nested and outer
allocations and returns S_OK regardless of that write result.
Method's semantic Win32 name is NOT inferred.

New standalone oem_write_data.[ch], oem_slot8_finish.[ch],
unit tests and a raster-stream integration regression were
added without reimplementing raster encoder or cutter logic.
A new source-verifier script checked 21 independent x86/x64
PE opcode sequences directly from the OEM Windows archive,
and cross-checked original Linux/macOS files. All passed.
GCC, Clang, ASan/UBSan passed locally. Native COM ABI,
real spooler and hardware execution remain open.


## 2026-10-18: x86/x64 original GDI query before media initialization

Prior check: branch sources HEAD
`efeef6833a83ab3baafe94bcf4d914cd0bd923e8`,
previous CI green, no existing original GDI probing module.
All three original manufacturer archives were re-read. ZIP SHA256s
Windows `cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca`,
Linux `54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17`,
macOS `6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210`
and internal original LeitzRp x86/x64 and Linux/macOS
raster2printer hashes match recorded original evidence.

Both original Windows LeitzRp DLLs execute
CreateDCW(WINSPOOL, printer_name, NULL, DEVMODE), read 13
GetDeviceCaps selectors in identical order
[10,8,10,4,6,88,90,40,42,110,111,112,113], and call
DeleteDC before feeding HORZRES/VERTRES into their layout
decisions. Read-only original verifier
`analysis/windows-2018/verify_original_rp_gdi_caps.py`
confirmed all 26 native selector immediates, four GDI import-call
opcode sequences and the two UTF-16 WINSPOOL literals, and rechecked
both other original platforms. No original binary was executed.

A **new**, separate `LeitzRp/oem_gdi_caps.[ch]` models only
this original GDI probe and forwards the measured dimensions to
existing `label_setup.c` via a callback-supplied DC, avoiding any
reimplementation of ModeAuto, orientation, endless media length,
RLE or cutter. A new C regression exercises the actual
preexisting label initializer in GitHub CI. Local GCC/Clang and
ASan/UBSan passed. The production branches and device remain
unchanged; real Unidrv invocation and printing not claimed.


## 2026-10-18: original raster-plugin GetPrinterW name retrieval

Duplicate-work audit: branch sources HEAD `6e21cdb66e5b1ffe540060184337d7fdcd6258e6`, latest CI
successful, and both original 2018 LeitzRp.dll architectures
from the unchanged Windows OEM ZIP available. Linux/macOS original
ZIPs and their actual raster2printer files were hashed again.

Previously recovered GDI CreateDCW expects a printer name as
a caller argument. The original raster COM vtable method 7
(x86 0x1370, x64 0x1350) constructs it via two
GetPrinterW(level=1/PRINTER_INFO_1W) calls: first NULL buffer and
0 bytes, ignore first BOOL; query pcbNeeded; allocate exactly
that temporary size, query again; on successful second BOOL copy
UTF-16 pName (x86 +8, x64 +16). Original method returns zero.
Exact x86/x64 opcode verification (10 independent patterns each)
and all three original archive hashes passed locally.

New `LeitzRp/oem_printer_name.[ch]` captures the safe,
caller-supplied callback/buffer form of the original two-query
decision; failed and short-buffer paths do not write output.
New unit tests and a connected `oem_printer_name_gdi_test.c`
feed this OEM-derived printer name to existing GDI/media
initialization and verify actual geometry and state without
duplicating earlier code. Strict GCC/Clang plus ASan/UBSan passed
locally. Original verification source:
`analysis/windows-2018/verify_original_rp_printer_name.py`.
No changes to production branches or the physical device;
native Windows COM/spooler execution remains unverified.


## Original Windows OEM raster slots 4/6: proven LeitzDevMode handoff

Verified preflight: sources HEAD `63c529820da24f3fc114a7f6af8aed9531fc6501`,
last Windows CI successful. All three original manufacturer
archives were rehashed (Linux 1.0.3, 2018 Windows and macOS
5.1.465), plus the actual Windows x86/x64 LeitzRp.dll and
LeitzDevMode.dll and Linux/macOS raster2printer. Original
machine-code audit confirms raster vtable slot 4 x86 RVA
0x15d0 and x64 0x15f0 directly delegate to the distinct
LeitzDevMode.dll export hrOEMDevMode. Native slot 6
x86 0x1340 and x64 0x1320 checks, releases and clears
the cached IUnknown host pointer but does not free the
raster object or alter its DLL-global lifetime count.

New isolated oem_unidrv_bridge.[ch] makes both already
recovered C functions callable from the appropriate original
raster vtable boundary: existing LeitzDevMode/oem_devmode
for private DEVMODE modes 1..4, and existing
LeitzRp/oem_host cached-interface Release for slot 6.
The bridge is explicit about normalized portable arguments,
not a native Windows COM object or ABI. GCC, Clang, ASan
and UBSan passed with local fixture equivalence; branch CI
will use the real repository modules. The manufacturer
source verifier confirms 9+8 exact x86/x64 machine opcode
sequences from original DLLs, importing/exporting names
and original three-ZIP cross-platform SHA256. No existing
reconstruction or production code overwritten; physical
device and real native Unidrv still untested.
