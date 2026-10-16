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
