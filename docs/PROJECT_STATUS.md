# Projektstand Leitz Icon Linux — 11.10.2026

> **Maßgeblicher Entwicklungsbranch:** `port/vendor-parity`. **Produktiv freigegebener Treiber: keiner.** `main` enthält den älteren experimentellen v0.4.1-Stand. Erfolgreiche GitHub Actions bestätigen Offline-Prüfungen, nicht die mechanische Fertigstellung eines Ausdrucks. Dieser Bericht trennt Quellcode, CI und reale Messungen.

## Orientierung

| Ort | Funktion | Stand |
| --- | --- | --- |
| `main` | Älterer, separat erhaltener experimenteller Stand | Nicht die aktuelle Portierung |
| `port/vendor-parity` | Integrierte laufende Entwicklung | Maßgeblich für neue Änderungen |
| `src/leitz_cups_filter.c` | CUPS-Raster, Medienabgleich, Auftragslänge | Ein gemeinsamer Pfad |
| `src/leitz_job.c` | Seitenfolge, gemeinsames `LabelCutter`-Verhalten, Encoderwahl | Implementiert |
| `src/leitz_codec.c` | `Current`: bisher hardwaregetestetes Protokoll | PPD-Standard |
| `src/leitz_native_reference.c` | `NativeLinux103`: originalnaher Linux-1.0.3-Rahmen | Experimentell, kein realer Drucktest |
| `src/leitz_native_job_validate.c` / `src/leitz_codec.c` | Separate Dokumentenvalidatoren | Offline getestet |
| `cups/leitzusb` | Prüfung vor USB-Übertragung, Kassettenstatus, Einmalübertragung | Keine während/nach dem Druck gemessene Fertigstellung |
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
| `NativeLinux103`, mehrere Seiten, Cutter ON/OFF | Implementiert und offline validiert | Kein Hardwaretest |
| `ESC U` Kassettenabfrage | Implementiert | Reale ASCII/SKU-Antworten gemessen |
| `ESC A` Bereitschaft vor Versand | Implementiert | Reale Einzelantwort `0x03` im Leerlauf; vollständiger Druck-Preflight noch nicht separat validiert |
| `ESC V` Diagnose | Im manuellen Probe-Werkzeug | 34-Byte-Textantwort real gemessen; Feldsemantik unbekannt |
| `ESC P` Fortschrittsabfrage | Nur manueller Diagnoseversuch | Im Leerlauf 0 Antwortbytes und Timeout; aktive Druckphase unbekannt |
| 86 Papierprofile | In PPD, JSON, Header synchron | Nur strukturell/offline; keine umfassenden Materialtests |
| Docker-CUPS Image | GitHub Actions baut es | Keine bestätigte Hardwarefreigabe für aktuellen Entwicklungsstand |

Bei der **gelben 88-mm-Kassette** wurden **49–51 mm statt 60 mm** beobachtet; ein späterer 100-mm-Test blieb nach Teil der ersten Druckzeile stehen. Die Ursache ist nicht bewiesen. Der Treiber darf den Auftrag bei unsicherem Übertragungsstand nicht blind wiederholen.

## Noch offen / priorisiert

1. **Mechanische Fertigstellung:** Der Backend-Erfolg bedeutet lediglich, dass USB-Schreibvorgänge abgeschlossen wurden, nicht dass Papiertransport und Schnitt nachweisbar beendet wurden. CUPS kann bereits `idle` melden. Während des Drucks fehlt validierte Status- und Fortschrittslogik.
2. **Gelbe Kassette:** Reproduzierte Längenabweichung und Druckabbruch; keine belastbare Ursache. Änderungen an Vorschub, Endmarkern oder Protokoll ohne Vergleichsdaten vermeiden.
3. **Original-Linux-Protokoll:** `NativeLinux103` und beide Cuttermodi müssen kontrolliert physisch mit dem Standard verglichen werden; durch originalgetreu nicht ausgegebene Leerzeilen können Längenabweichungen entstehen.
4. **Breiten und Medien:** Zusätzliche SKU, gestanzte Etiketten und abweichende A/B-Geometrien physisch prüfen; die SKU-Breitenzuordnung im Backend ist bislang nicht umfassend am Gerät nachgewiesen.
5. **Herstellerparität:** CUPS-Befehlskanal, Status-/Fehlerbehandlung, herstellerspezifische Rasteralgorithmen, Margin-Semantik, Windows-64-Byte-Sonderfall und sichere Recovery sind noch nicht vollständig implementiert beziehungsweise geklärt. Der neue 8-Bit-Graustufenpfad bietet wählbar Bayer4 und Schwellwert-Rasterung; beide sind ausschließlich offline getestet und nicht mit den proprietären macOS-Rasteralgorithmen gleichgesetzt.
6. **Produktionsfreigabe:** Reproduzierbare End-to-End-Druckprüfung, installierbarer versionierter Build und belastbare Unraid-Anleitung fehlen; derzeit kein Release.
7. **Veröffentlichung und Lizenz:** Im Repository ist noch keine Lizenzdatei und keine explizit gewählte Weiterverwendungslizenz vorhanden. Ein Release benötigt eine bewusste Lizenzentscheidung; öffentlich lesbarer Quellcode ist nicht automatisch frei lizenzierter Code.
8. **Repo-Organisation:** `main` bleibt vorerst als v0.4.1-Vergleichsbasis bestehen. Die Entwicklung wird über einen **Draft-PR gegen `main`** sichtbar; erst nach gebündelter Hardwarefreigabe zusammenführen und veröffentlichen.

## Konkrete Arbeitsreihenfolge

1. Alle Quell-/CI-/Dokumentationsfehler offline gebündelt beheben und überprüfen; den bereits getesteten Standard-Encoder unverändert erhalten.
2. Die ungeklärte gelbe Kassette, die beiden Protokolle, feste/variable Länge, Aufträge mit 1/2+ Etiketten, beide Auflösungen und Schnittmodi in **einer koordinierten Hardwaretestreihe** vergleichen.
3. Danach ein nachweislich kompatibles Standardprotokoll bestimmen; unbestätigte Alternativen und Diagnosewerkzeuge bis dahin behalten. Erst dann `main`-Zusammenführung, Release, Unraid-Installation freigeben.

**Nicht behaupten:** 86 physisch funktionierende Medien; `ESC P`-Fertigstellungsbestätigung; erfolgreichen Druck nur aufgrund von `cat`/USB-Write; eine bekannte Ursache für die gelbe Kassette; identischen Status aller Geräte/Firmwares.
