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
