# Reproduzierbarer Vier-Quellen-Abgleich mit Original-Windows-GPD

Der bestehende Bericht `PAPER_FORMATS_CROSS_SOURCE.md` enthält 40 Linux-, 41 macOS- und 86 Entwicklungsprofile. Windows enthält gemäß der früher verifizierten Original-GPD **77 Optionen** (76 feste und `CUSTOMSIZE`); bisher konnten nur `DHLShippingLabel` und `CUSTOMSIZE` aus den dokumentierten Befunden namentlich angeführt werden. Der vorliegende Zusatz ersetzt den unvollständigen Windows-Teil **nicht durch Annahmen**: Er liest alle GPD-Optionen direkt aus dem ursprünglichen Hersteller-ZIP und erzeugt daraus einen vollständigen Vier-Spaltenvergleich samt Windows-OptionIDs, Ressourcenkennungen, Auswahlbefehlen und Geometriedaten.

## Voraussetzungen / Ausführung

Im Branch `sources` (Repositorywurzel), mit separat bereitgestelltem Entwicklungskatalog aus dem Branch `driver`:

```sh
python3 analysis/windows-2018/compare_windows_gpd_four_sources.py \
  /PFAD/LeitzIconLabelSetup\(4\).zip \
  /PFAD/media.json > /PFAD/windows_four_sources.md
```

Die beiden Pfade müssen auf tatsächlich vorhandene Dateien verweisen. Es wird **nichts installiert oder am Drucker ausgeführt**. Die ZIP wird ausschließlich lesend geöffnet, ohne Installer/PE/EXE zu starten. Die GPD wird gegen SHA-256 `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7` geprüft. Bei fehlendem oder falschem Original schlägt das Programm fehl, statt fiktive Windows-Einträge zu ergänzen.

Die Ausgabedatei entsteht erst bei bereitgestelltem, exakt passendem Original. Sie ist **nicht** als bereits erzeugter, vollständiger Windows-Abgleich im Git enthalten, weil die ursprüngliche ZIP in dieser Sitzung nicht als Binärdatei zugänglich war.

## Bedeutungen / Sicherheitsgrenzen

- **Lesbare Namen:** Exakte PPD-Bezeichnungen für Linux/macOS und der Entwicklungskatalog. Die Windows-GPD liefert interne `*Option`-Namen und numerische `rcNameID`-Referenzen; sie liefert **keine** aufgelösten lokalisierten Klartextnamen. Diese werden als fehlend behandelt.
- **Windows Originalnamen:** Beibehalten; nur wortgleiche IDs werden in einer Zeile vereint. Es erfolgt keine Zusammenlegung anhand ähnlicher Maße, SKU-Strings oder Alias-Namen.
- **Geometrie:** Bei wortgleicher ID werden Windows-GPD `PageDimensions`, `PrintableArea` und `PrintableOrigin` mit `page`, `area` und `origin` aus der Eigenentwicklung verglichen. Diese Felder verwenden im vorhandenen Datenmodell die GPD-MasterUnits `(300,600)`. Gleichheit der Zahlen beweist nicht die Gleichheit der Papierkassette.
- **Variable Länge:** `CUSTOMSIZE` besitzt keine feste `PageDimensions` und wird nicht als 170-mm-Material oder mit einem pauschalen Längenmaximum gleichgesetzt.
- **Gerätefunktionen:** Der Bericht sendet keinerlei Druck-/Schnittbefehle und ersetzt weder Unidrv, CUPS noch den bestehenden Rasterfilter. Die tatsächliche Windows-PPD/GPD-Unidrv-Interaktion bleibt separat offen.

## Regression

`python3 -m unittest discover -s analysis/windows-2018 -p 'test_compare_windows_gpd_four_sources.py' -v`

Die Regression nutzt **ausdrücklich synthetische Testoptionen** und prüft Quelltrennung, Bytefolgen-Darstellung, fehlende Übersetzungen, Geometriedifferenzen, Duplikate und zwingende Original-Hashprüfung. Sie bestätigt **nicht** die Inhalte der 75 früher nicht inventarisierten Windows-Einträge. Die Windows-Rekonstruktions-CI testet diesen Zusatz unabhängig von den übrigen Modulen.
