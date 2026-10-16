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
