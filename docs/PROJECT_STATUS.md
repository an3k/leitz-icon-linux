# Projektstand Leitz Icon Linux — 11.10.2026

> **Maßgeblicher Entwicklungsbranch:** `port/vendor-parity`. **Produktiv freigegebener Treiber: keiner.** `main` enthält den älteren experimentellen v0.4.1-Stand. Erfolgreiche GitHub Actions bestätigen Offline-Prüfungen, nicht die mechanische Fertigstellung eines Ausdrucks. Dieser Bericht trennt Quellcode, CI und reale Messungen.

## Orientierung

| Ort | Funktion | Stand |
| --- | --- | --- |
| `main` | Älterer, separat erhaltener experimenteller Stand | Nicht die aktuelle Portierung |
| `port/vendor-parity` | Integrierte laufende Entwicklung | Maßgeblich für neue Änderungen |
| `src/leitz_cups_filter.c` | CUPS-Raster, Medienabgleich, Auftragslänge | Ein gemeinsamer Pfad |
| `src/leitz_job.c` | Seitenfolge, gemeinsames `LabelCutter`-Verhalten, Encoderwahl | Implementiert |
| `src/leitz_protocol_current.c` | `Current`: komplette Kodierung und Validierung in einer optionalen Datei | Standard bei vorhandenem Modul |
| `src/leitz_protocol_native_linux_103.c` | `NativeLinux103`: komplette Kodierung und Validierung in einer optionalen Datei | Experimentell, kein realer Drucktest |
| `src/leitz_protocol.h`, `src/leitz_protocol_registry.c` | Einheitliche VTable-Schnittstelle und Modulregistrierung ohne Protokollkodierung | Einzelmodul-Builds werden offline geprüft |
| `cups/leitzusb` | Prüfung vor USB-Übertragung, Kassettenstatus, Einmalübertragung, ESC A für CUPS-ReportStatus | Keine während/nach dem Druck gemessene Fertigstellung |
| `cups/leitz-command` | Eng begrenzter CUPS-Befehlskanal: nur `ReportStatus`, ohne Druck-/Schnittdaten | Nur offline geprüft; live noch nicht erprobt |
| `cups/entrypoint.sh`, `cups/Dockerfile` | CUPS-Docker/Unraid-Bereitstellung | Image in CI erstellt; Betrieb des aktuellen Builds auf TARS nicht bestätigt |
| `cups/LeitzIcon.ppd`, `media.json`, `src/leitz_media.h` | Papierprofile, Formate, Geometrien | Katalogkonsistenz offline geprüft |
| `src/leitz_probe*.c`, `src/leitz_status.c` | Manuelles Statuswerkzeug und Offline-Dekoder | Einzelne echte Antworten bekannt, keine Laufzeitüberwachung |
| `tests/`, `tools/check_media_consistency.py` | Offline-Regressionen | Keine Ersatzprüfung für Hardware |
| `docs/PORTING_STATUS.md` | Detaillierte, historisch fortgeschriebene Portierungsnotizen | Einzelne ältere Aussagen darin nur im jeweiligen Zeitkontext lesen |

Vor der Bestandsaufnahme war `main` bei `1afdab16`, `port/vendor-parity` bei `2fb8a9d9` (**187 Commits voraus, 0 zurück**). Auf diesem Entwicklungsstand meldete GitHub Actions `38125541659` Erfolg für Build, Tests, PPD und Docker. Der Bericht selbst und folgende Korrekturen erzeugen weitere Commits.

## Originalquellen und Medien

Die bisherige Auswertung der Herstellerquellen dokumentiert **40** benannte Linux-1.0.3-PPD-Formate, **41** macOS-1.2.190-PPD-Formate sowie **76** Windows-GPD-Optionen (**75 feste + `CUSTOMSIZE`**). Die jeweiligen Quellen sind **nicht im öffentlichen Repository vollständig enthalten**. Eine erneute Vollverifikation der Herstellerarchive benötigt diese Originaldaten; ihre bisherige Auswertung ist in `docs/PORTING_STATUS.md` und `docs/VENDOR_LINUX_PROTOCOL.md` festgehalten.

Der aktuelle Entwicklungskatalog hat **86 PPD-Profile**: 43 nichtpräfixierte A-Profile und 43 `Leitz`-präfixierte B-Profile; darunter **64 feste und 22 Endlos-Profile**. Die Profile sind keine Aussage über 86 physisch unabhängige Medien. Vier A/B-Geometrien unterscheiden sich nach Quellenlage bewusst: `LargePendaflexTabRotated`, `Multipurpose70030001Rotated`, `NameBadgeCard70190001Rotated`, `ShippingLabel70180001Rotated`. 39 B-Anzeigenamen stammen aus Windows-Ressourcen, vier sind abgeleitete Bezeichnungen; siehe `tools/media_b_original_labels.json`.

Papierkassette/SKU, physische Breite, gewähltes CUPS-Papierformat und tatsächlich bedruckbare Fläche sind **vier verschiedene Größen**. 170 mm ist ausschließlich eine historische Testlänge, kein Treiberlimit. Die maximale Länge hängt von CUPS-Seite, Auftragsinhalt, Encodergrenzen und Material ab; Endlosmaterial wird in den bekannten Betriebsarten nach Inhalt geschnitten.

## Getestet und ungeprüft

| Eigenschaft | Softwarestand | Reale Evidenz |
| --- | --- | --- |
| `Current` USB-Druck auf weißer 88-mm-Endlosrolle | Vorhanden | 300×300 und 300×600 dpi, benutzerdefinierte Länge, mehrseitige Aufträge und automatischer Schnitt bereits getestet |
| `Current` als Standard / gemeinsamer Jobcontroller | Implementiert | Refaktorierte/neue Teilpfade nicht erneut vollständig am Gerät geprüft |
| `Current` und `NativeLinux103`, Mehrseitenaufträge, Cutter ON/OFF | Ein gemeinsamer Controller verlangt dieselbe Breite/Auflösung für alle Seiten, erlaubt unterschiedliche Drucklängen bei Endlosmaterial. Beide Spoolvalidatoren prüfen die Breite erneut. | Neue Integrationspfade nicht vollständig hardwaregetestet |
| `ESC U` Kassettenabfrage | Implementiert | Reale ASCII/SKU-Antworten gemessen |
| `ESC A` Bereitschaft vor Versand | Implementiert | Reale Einzelantwort `0x03` im Leerlauf; vollständiger Druck-Preflight noch nicht separat validiert |
| `ESC V` Diagnose | Im manuellen Probe-Werkzeug | 34-Byte-Textantwort real gemessen; Feldsemantik unbekannt |
| `ESC P` Fortschrittsabfrage | Nur manueller Diagnoseversuch | Im Leerlauf 0 Antwortbytes und Timeout; aktive Druckphase unbekannt |
| 86 Papierprofile | In PPD, JSON, Header synchron | Nur strukturell/offline; keine umfassenden Materialtests |
| Docker-CUPS Image | GitHub Actions baut es, einschließlich neuem Status-Kommandofilter | Keine bestätigte Hardwarefreigabe für aktuellen Entwicklungsstand |

Bei der **gelben 88-mm-Kassette** wurden **49–51 mm statt 60 mm** beobachtet; ein späterer 100-mm-Test blieb nach Teil der ersten Druckzeile stehen. Die Ursache ist nicht bewiesen. Der Treiber darf den Auftrag bei unsicherem Übertragungsstand nicht blind wiederholen.

## Noch offen / priorisiert

1. **Mechanische Fertigstellung:** Der Backend-Erfolg bedeutet lediglich, dass USB-Schreibvorgänge abgeschlossen wurden, nicht dass Papiertransport und Schnitt nachweisbar beendet wurden. CUPS kann bereits `idle` melden. Während des Drucks fehlt validierte Status- und Fortschrittslogik.
2. **Gelbe Kassette:** Reproduzierte Längenabweichung und Druckabbruch; keine belastbare Ursache. Änderungen an Vorschub, Endmarkern oder Protokoll ohne Vergleichsdaten vermeiden.
3. **Original-Linux-Protokoll:** `NativeLinux103` und beide Cuttermodi müssen kontrolliert physisch mit dem Standard verglichen werden. Der native Encoder bleibt beim ursprünglichen Verhalten ohne Ausgabe abschließender Leerzeilen. Ein zuvor im gemeinsamen Controller ergänzter, nur nativer Vorschub wurde zurückgenommen; mögliche Längenabweichungen bleiben bis zum Hardwarevergleich offen.
4. **Breiten und Medien:** Zusätzliche SKU, gestanzte Etiketten und abweichende A/B-Geometrien physisch prüfen; die SKU-Breitenzuordnung im Backend ist bislang nicht umfassend am Gerät nachgewiesen.
5. **Herstellerparität:** Der CUPS-Befehlskanal unterstützt jetzt `ReportStatus` (ESC A) über den exklusiven Backend-USB-Zugang, aber noch keine weitergehende Liveüberwachung. Status-/Fehlerbehandlung, herstellerspezifische Rasteralgorithmen, Margin-Semantik, Windows-64-Byte-Sonderfall und sichere Recovery sind noch nicht vollständig implementiert beziehungsweise geklärt. Der neue 8-Bit-Graustufenpfad bietet wählbar Bayer4 und Schwellwert-Rasterung; beide sind ausschließlich offline getestet und nicht mit den proprietären macOS-Rasteralgorithmen gleichgesetzt.
6. **Produktionsfreigabe:** Reproduzierbare End-to-End-Druckprüfung, installierbarer versionierter Build und belastbare Unraid-Anleitung fehlen; derzeit kein Release.
7. **Veröffentlichung und Lizenz:** Im Repository ist noch keine Lizenzdatei und keine explizit gewählte Weiterverwendungslizenz vorhanden. Ein Release benötigt eine bewusste Lizenzentscheidung; öffentlich lesbarer Quellcode ist nicht automatisch frei lizenzierter Code.
8. **Repo-Organisation:** `main` bleibt vorerst als v0.4.1-Vergleichsbasis bestehen. Die Entwicklung wird über einen **Draft-PR gegen `main`** sichtbar; erst nach gebündelter Hardwarefreigabe zusammenführen und veröffentlichen.

## Konkrete Arbeitsreihenfolge

1. Alle Quell-/CI-/Dokumentationsfehler offline gebündelt beheben und überprüfen; den bereits getesteten Standard-Encoder unverändert erhalten.
2. Die ungeklärte gelbe Kassette, die beiden Protokolle, feste/variable Länge, Aufträge mit 1/2+ Etiketten, beide Auflösungen und Schnittmodi in **einer koordinierten Hardwaretestreihe** vergleichen.
3. Danach ein nachweislich kompatibles Standardprotokoll bestimmen; unbestätigte Alternativen und Diagnosewerkzeuge bis dahin behalten. Erst dann `main`-Zusammenführung, Release, Unraid-Installation freigeben.

**Nicht behaupten:** 86 physisch funktionierende Medien; `ESC P`-Fertigstellungsbestätigung; erfolgreichen Druck nur aufgrund von `cat`/USB-Write; eine bekannte Ursache für die gelbe Kassette; identischen Status aller Geräte/Firmwares.

## Modularisierung der Protokolle (11.10.2026)

Die zwei Druckprotokolle liegen isoliert in genau je einer Implementierungsdatei:
`src/leitz_protocol_current.c` und `src/leitz_protocol_native_linux_103.c`.
Beide enthalten ihre eigenen Kodierer **und** Binärvalidatoren. Die gemeinsame
Schnittstelle liegt in `src/leitz_protocol.h`, die schwache Modulregistrierung
in `src/leitz_protocol_registry.c`. Der gemeinsame Jobcontroller enthält
keine protokollspezifischen Raster-/Medien-/Vorschubregeln. Die Wire-Frames
sind wegen der verschiedenen Herstellerprotokolle absichtlich unterschiedlich.

`Makefile` verwendet nur vorhandene Protokollquellen. `tools/build_ppd.py`
generiert daraus `dist/LeitzIcon.ppd`, das im Docker-Image installiert wird.
Bei nur einem vorhandenen Protokoll verschwindet das andere aus den Binärdateien
und aus der CUPS-Auswahl. Tests: `tests/test_protocol_isolation.py`
(jeweils ein Quellmodul physisch entfernt, neu kompiliert, PPD gefiltert und
Protokollauswahl kontrolliert). Diese Prüfung ist **offline** und belegt keine
vollständige mechanische Ausgabe am Drucker.
