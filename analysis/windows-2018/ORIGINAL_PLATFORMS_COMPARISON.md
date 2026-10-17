# Herstellerpfade Linux 1.0.3, Windows 2018 und macOS Classic 5.1.465

## Originalquellen und Grenzen der Prüfung

Die Archive LeitzIconDriversForLinux1.0.3.zip,
LeitzIconLabelSetup.zip und LeitzIconSoftware-Installer_5.1.465.dmg.zip
sind in der Projektbibliothek aufgeführt. Ihre originalen ZIP-Bytes konnten
in dieser Sitzung nicht materialisiert werden. Die bereits gesicherten
Originaldateien und verifizierten Rekonstruktionsbefunde sind dagegen im
Repository oder als lesbare GPD-Transkription verfügbar.

- Linux: unveränderte Originalquellen raster2printer.c,
  raster_c_functions.c, Header und LeitzIcon.ppd im Branch sources.
- Windows: aus LeitzIconLabelSetup(4).zip zuvor extrahierte GPD,
  vollständig erhaltenes Inventar aller 77 PaperSize-Einträge;
  Original-DLLs bislang nur durch frühere statische Befunde untersucht.
- macOS: unveränderte Original-PPD, zuvor an Original-Mach-O-Code
  abgeglichene spezifische Printfunktionen unter
  reconstructions/macos-5.1.465-classic/raster2printer/.
  Diese C-Module sind Rekonstruktionen, keine OEM-Original-C-Dateien.

## Unterschiedliche Herstellerbefehle

| Funktion | Linux 1.0.3 | Windows GPD 2018 | macOS Classic (Original-Binärbefunde) |
| --- | --- | --- | --- |
| Dokumentbeginn | ESC @, danach ESC h oder ESC i, zusammen 4 Byte | CmdStartDoc: 121 ESC, danach @ (122 Byte); Auflösung separat | 120 ESC, dann ESC @ (122 Byte), optional ESC i |
| Start einer Seite | Linux-CUPS-Filter bereitet Raster/Geometrie auftragsbezogen vor | je PaperSize CmdSelect: ESC L Länge, ESC w Breite; CmdStartPage: ESC B 00 | ESC L Länge, ESC w Breite, ESC B 00; Werte von der Rasterseite |
| Seitenabschluss | ESC G, je Option ESC E | LeitzRp: ESC G und optional ESC E, nach bereits rekonstruiertem Zeilenzustand | ESC G, ggf. ESC E |
| Dokumentende | ESC G und ESC E | CmdEndJob: ESC E, ESC @, ESC Z | ggf. ESC E, immer ESC @, ESC Z |
| Papierprofile | 40 PPD-PageSize-Einträge | 76 feste GPD-Optionen plus CUSTOMSIZE | 41 PPD-PageSize-Einträge |

**Nicht austauschbar:** Windows-GPD-Befehle werden von Unidrv und
dem separaten LeitzRp-Rasterplugin verarbeitet; die tatsächliche
hostseitige Kombination und Reihenfolge wurde nicht durch diese
statischen Quellen rekonstruiert. Auch die 120/121 ESC-Folgen aus
verschiedenen Quellen dürfen nicht blind gleichgesetzt werden.

## Neues Windows-2018-GPD-Medienmodul

Der vollständig aus Original-GPD-Daten generierte portable
gpd_media_catalog.[ch] enthält 77 Originalnamen, 76 feste
OptionIDs/rcNameIDs, geometrische Werte, CUSTOMSIZE-Grenzen
und die jeweils unveränderten ESC-L/ESC-w-Befehlsbytes.

- 20 Windows-Endlosformate benutzen den Original-Längenwert FFFF.
- AppointmentCard und LeitzAppointmentCard besitzen im Original
  CmdSelect den Längenwert 570, obwohl ihre GPD-Seitenhöhe 1200 ist.
  Eine allgemeine Ableitung der Befehlslänge als Seitenhöhe/2 wäre falsch.
- Die Windows-GPD beschreibt CUSTOMSIZE ohne feste Höhe mit
  MinSize=(16,16) und MaxSize=(10000,64000) in GPD-MasterUnits.
  Daraus folgt keine pauschale 170-mm-Begrenzung.
- Die 76 festen GPD-IDs besitzen je einen gleichnamigen Eintrag im
  Entwicklungskatalog und identische dort gespeicherte Geometriewerte.
  Übereinstimmung begründet keine mechanische Kassettenidentität.

Reproduzierbarkeit: Die vollständige Original-GPD-Extraktion und eine
Texttranskription stehen nur unter analysis/windows-2018, **nicht**
unter originals, denn bei der Textübernahme sind Zeilenenden
normalisiert. Die zuvor verifizierte SHA-256 der OEM-Datei
f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7
wird als Quellbeleg dokumentiert, nicht als aktuelle Hashsumme der
Texttranskription ausgegeben.

Der Generator erzeugt die C-Tabelleneinträge ausschließlich aus den
77 GPD-Blöcken nach Abgleich mit dem gesicherten JSON. Die CI prüft
C-Tabellen-Synchronität, alle 77 GPD-Datensätze, 76 Geometrien
gegen den unveränderten Entwicklungskatalog und die 87-Zeilen-
Vergleichstabelle. Keine Windows-Ressourcen-Anzeigenamen erfunden.

## Noch offen

Authentisches Windows-COM-/Unidrv-Verhalten und dessen Datenfluss
bis zum USB-Gerät; lokalisierte Windows-Klartextnamen aus der
Original-Ressourcen-DLL; tatsächlicher Druck, automatische Schnitte,
gerätespezifische Fehler-/Statusantworten sowie dynamische
Windows-vs-Linux/macOS-Ausgabevergleiche.

Keine neuen Gerätetests, Installerstarts, Firmwareänderungen oder
Änderungen der produktiven Branches.
