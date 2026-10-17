# Vollständiger Abgleich der Windows-Papierformatnamen (77 GPD-Optionen)

**Quellenstand:** Windows-GPD und bereits erstellte 77er-GPD-Inventur wurden am 17.10.2026 aus der Projektbibliothek gelesen und alle 77 Datensätze anhand des lesbaren GPD-Inhalts Feld für Feld bestätigt. Die Rohdatei und vollständige Inventur konnten anschließend nicht materialisiert werden; diese Tabelle hält die nachgewiesenen **originalen Windows-IDs**, nicht die ungesicherten Ressourcenstrings oder die vollständigen GPD-Datensätze fest.

**Originalbeleg:** `LeitzIcon.gpd`, archivierte SHA-256 `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`, Windows-Treiberversion 2.17.4.112. Die SHA-256-Rohbyteprüfung wurde in dieser Sitzung **nicht erneut** ausgeführt; eine UTF-8-Textansicht ersetzt keine Byteprüfung.

**Anzahlen:** Linux 40, macOS Classic 41, Windows 77 (76 fest + `CUSTOMSIZE`), Linux-Eigenentwicklung 86; Vereinigungsmenge 87 IDs. **76 von 76 festen Windows-IDs** sind im Entwicklungskatalog vorhanden. Ihre `PageDimensions`, `PrintableArea`, `PrintableOrigin` stimmen als gespeicherte Werte je ID überein. Die zehn nur in der Eigenentwicklung stehenden IDs sind im Abschnitt unten aufgelistet.

**Wichtig:** Die Windows-Klartextanzeige ist in der GPD nur als `rcNameID` referenziert und hier nicht aufgelöst. Die Spalte „Entwicklungslabel“ ist **kein** originales Windows-Anzeigelabel. GPD-Formatgröße und bedruckbare Fläche beweisen keine physische Materialkassette, Kassettenbreite oder Schnittfunktion. `CUSTOMSIZE` ist variabel; seine GPD-Grenzen `[16,16]` und `[10000,64000]` (MasterUnits `300×600`) sind keine allgemeine feste Drucklänge.

## Nebeneinanderstellung – exakte interne IDs

Die Geometriezahlen stammen aus dem separat geprüften Entwicklungskatalog (`page / area / origin`, GPD MasterUnits 300×600); bei den 76 gleichnamigen Windows-Festformaten wurde ihre Übereinstimmung mit der GPD bestätigt. Keine Gleichsetzung anhand ähnlich klingender Namen.

| Interne ID | Linux Original-Anzeige | macOS Original-Anzeige | Windows Original-ID | Windows OptionID | Entwicklungslabel | Geometrie Page / Area / Origin |
| --- | --- | --- | --- | --- | --- | --- |
| `AppointmentCard` | Appointment Card | Appointment Card | `AppointmentCard` | nicht gesichert | Appointment Card | 1075 × 1200 / 960 × 1140 / 58 × 30 |
| `CUSTOMSIZE` | — | — | `CUSTOMSIZE` | — | — | — |
| `Continuous112` | Continuous - 1 1/2" | Continuous - 1 1/2" | — | nicht gesichert | Continuous - 1 1/2" | — |
| `Continuous70030001` | Continuous - 3 1/2" | Continuous - 3 1/2" | `Continuous70030001` | nicht gesichert | Continuous - 3 1/2" | 1039 × 64000 / 960 × 64000 / 40 × 0 |
| `Continuous70040001` | Continuous - 2 2/5" | Continuous - 2 2/5" | `Continuous70040001` | nicht gesichert | Continuous - 2 2/5" | 720 × 64000 / 690 × 64000 / 15 × 0 |
| `Continuous70050001` | Continuous - 2 1/5" | Continuous - 2 1/4" | `Continuous70050001` | nicht gesichert | Continuous - 2 1/4" | 673 × 64000 / 643 × 64000 / 15 × 0 |
| `Continuous70060001` | Continuous - 2" | Continuous - 2" | `Continuous70060001` | nicht gesichert | Continuous - 2" | 591 × 64000 / 561 × 64000 / 15 × 0 |
| `Continuous70070001` | — | — | `Continuous70070001` | nicht gesichert | Continuous - 1 1/2" | 461 × 64000 / 431 × 64000 / 15 × 0 |
| `Continuous70080001` | Continuous - 1 1/4" | Continuous - 1 1/4" | `Continuous70080001` | nicht gesichert | Continuous - 1 1/4" | 378 × 64000 / 348 × 64000 / 15 × 0 |
| `Continuous70090001` | Continuous - 1" | Continuous - 1" | `Continuous70090001` | nicht gesichert | Continuous - 1" | 295 × 64000 / 265 × 64000 / 15 × 0 |
| `Continuous70100001` | Continuous - 3/4" | Continuous - 3/4" | `Continuous70100001` | nicht gesichert | Continuous - 3/4" | 224 × 64000 / 194 × 64000 / 15 × 0 |
| `Continuous70110001` | Continuous - 1/2" | Continuous - 1/2" | `Continuous70110001` | nicht gesichert | Continuous - 1/2" | 148 × 64000 / 118 × 64000 / 15 × 0 |
| `Continuous70190001` | Continuous - 3 3/5" | Continuous - 3 3/5" | `Continuous70190001` | nicht gesichert | Continuous - 3 3/5" | 1075 × 64000 / 960 × 64000 / 57 × 0 |
| `DHLShippingLabel` | — | DHLShippingLabel | `DHLShippingLabel` | 335 | DHL Shipping Label | 1039 × 4204 / 960 × 4144 / 40 × 30 |
| `FileFolderLabel` | File Folder Label | File Folder Label | `FileFolderLabel` | nicht gesichert | File Folder Label | 148 × 2100 / 118 × 2040 / 15 × 30 |
| `InternationalAddressLabel` | International Address Label | International Address Label | `InternationalAddressLabel` | nicht gesichert | International Address Label | 1039 × 850 / 960 × 790 / 40 × 30 |
| `LargePendaflexTab` | Large Pendaflex Tab | Large Pendaflex Tab | `LargePendaflexTab` | nicht gesichert | Large Pendaflex Tab | 591 × 662 / 561 × 602 / 15 × 30 |
| `LargePendaflexTabRotated` | Large Pendaflex Tab Rotated | Large Pendaflex Tab Rotated | — | nicht gesichert | Large Pendaflex Tab Rotated | — |
| `LeitzAppointmentCard` | — | — | `LeitzAppointmentCard` | nicht gesichert | *Appointment Card | 1075 × 1200 / 960 × 1140 / 58 × 30 |
| `LeitzContinuous112` | — | — | — | nicht gesichert | *Continuous - 1 1/2" | — |
| `LeitzContinuous70030001` | — | — | `LeitzContinuous70030001` | nicht gesichert | *Continuous - 3.5" | 1039 × 64000 / 960 × 64000 / 40 × 0 |
| `LeitzContinuous70040001` | — | — | `LeitzContinuous70040001` | nicht gesichert | *Continuous - 2.4" | 720 × 64000 / 690 × 64000 / 15 × 0 |
| `LeitzContinuous70050001` | — | — | `LeitzContinuous70050001` | nicht gesichert | *Continuous - 2.25" | 673 × 64000 / 643 × 64000 / 15 × 0 |
| `LeitzContinuous70060001` | — | — | `LeitzContinuous70060001` | nicht gesichert | *Continuous - 2" | 591 × 64000 / 561 × 64000 / 15 × 0 |
| `LeitzContinuous70070001` | — | — | `LeitzContinuous70070001` | nicht gesichert | *Continuous - 1.54" | 461 × 64000 / 431 × 64000 / 15 × 0 |
| `LeitzContinuous70080001` | — | — | `LeitzContinuous70080001` | nicht gesichert | *Continuous - 1.25" | 378 × 64000 / 348 × 64000 / 15 × 0 |
| `LeitzContinuous70090001` | — | — | `LeitzContinuous70090001` | nicht gesichert | *Continuous - 1" | 295 × 64000 / 265 × 64000 / 15 × 0 |
| `LeitzContinuous70100001` | — | — | `LeitzContinuous70100001` | nicht gesichert | *Continuous - 0.75" | 224 × 64000 / 194 × 64000 / 15 × 0 |
| `LeitzContinuous70110001` | — | — | `LeitzContinuous70110001` | nicht gesichert | *Continuous - 0.5" | 148 × 64000 / 118 × 64000 / 15 × 0 |
| `LeitzContinuous70190001` | — | — | `LeitzContinuous70190001` | nicht gesichert | *Continuous - 3.6" | 1075 × 64000 / 960 × 64000 / 57 × 0 |
| `LeitzDHLShippingLabel` | — | — | — | nicht gesichert | *DHL Shipping Label | — |
| `LeitzFileFolderLabel` | — | — | `LeitzFileFolderLabel` | nicht gesichert | *File Folder Label | 148 × 2100 / 118 × 2040 / 15 × 30 |
| `LeitzInternationalAddressLabel` | — | — | `LeitzInternationalAddressLabel` | nicht gesichert | *Large Address Label | 1039 × 850 / 960 × 790 / 40 × 30 |
| `LeitzLargePendaflexTab` | — | — | `LeitzLargePendaflexTab` | nicht gesichert | *Large Pendaflex Tab | 591 × 662 / 561 × 602 / 15 × 30 |
| `LeitzLargePendaflexTabRotated` | — | — | `LeitzLargePendaflexTabRotated` | nicht gesichert | *Large Pendaflex Tab R | 331 × 1182 / 301 × 1122 / 15 × 30 |
| `LeitzLeverArchFile1005` | Active File | Active File | `LeitzLeverArchFile1005` | nicht gesichert | *Lever Arch File 1005 | 591 × 2598 / 561 × 2538 / 15 × 30 |
| `LeitzMultipurpose70030001` | — | — | `LeitzMultipurpose70030001` | nicht gesichert | *Multipurpose 2"x3.5" | 1039 × 1200 / 960 × 1140 / 40 × 30 |
| `LeitzMultipurpose70030001Rotated` | — | — | `LeitzMultipurpose70030001Rotated` | nicht gesichert | *Multipurpose 2"x3.5" R | 600 × 2078 / 570 × 2018 / 15 × 30 |
| `LeitzMultipurpose70090001` | — | — | `LeitzMultipurpose70090001` | nicht gesichert | *Multipurpose 1"x1.5" | 295 × 900 / 265 × 840 / 15 × 30 |
| `LeitzMultipurpose70100001` | — | — | `LeitzMultipurpose70100001` | nicht gesichert | *Multipurpose 0.75"x2.5" | 224 × 1500 / 194 × 1440 / 15 × 30 |
| `LeitzMultipurpose70110001` | — | — | `LeitzMultipurpose70110001` | nicht gesichert | *Multipurpose 0.5"x1.875" | 148 × 1134 / 118 × 1074 / 15 × 30 |
| `LeitzNameBadgeCard70190001` | — | — | `LeitzNameBadgeCard70190001` | nicht gesichert | *Name Badge Card 2.25"x3.5" | 1075 × 1350 / 960 × 1290 / 58 × 30 |
| `LeitzNameBadgeCard70190001Rotated` | — | — | `LeitzNameBadgeCard70190001Rotated` | nicht gesichert | *Name Badge Card 2.25"x3.5" R | 675 × 2078 / 645 × 2018 / 15 × 30 |
| `LeitzNameBadgeCard8214` | — | — | — | nicht gesichert | *Durable Label | — |
| `LeitzNameBadgeLabel70030001` | — | — | `LeitzNameBadgeLabel70030001` | nicht gesichert | *Name Badge 2.33"x3.5" | 1039 × 1398 / 960 × 1338 / 40 × 30 |
| `LeitzNameBadgeLabel70060001` | — | — | `LeitzNameBadgeLabel70060001` | nicht gesichert | *Name Badge Label 2"x3" | 591 × 1800 / 561 × 1740 / 15 × 30 |
| `LeitzNarrowLeverArchCardShort` | — | — | `LeitzNarrowLeverArchCardShort` | nicht gesichert | *Narrow Lever Arch Card Short | 378 × 4488 / 348 × 4428 / 15 × 30 |
| `LeitzNarrowLeverArchLabelLong` | — | — | `LeitzNarrowLeverArchLabelLong` | nicht gesichert | *Narrow Lever Arch Label Long | 461 × 6732 / 431 × 6672 / 15 × 30 |
| `LeitzNarrowLeverArchLabelShort` | — | — | `LeitzNarrowLeverArchLabelShort` | nicht gesichert | *Narrow Lever Arch Label Short | 461 × 4536 / 431 × 4476 / 15 × 30 |
| `LeitzPackageShippingLabel` | — | — | `LeitzPackageShippingLabel` | nicht gesichert | *Package Label 6"x3.5" | 1039 × 3600 / 960 × 3540 / 40 × 30 |
| `LeitzReturnAddressLabelLarge` | — | — | `LeitzReturnAddressLabelLarge` | nicht gesichert | *Return Address Label Large | 295 × 1276 / 265 × 1216 / 15 × 30 |
| `LeitzReturnAddressLabelSmall` | — | — | `LeitzReturnAddressLabelSmall` | nicht gesichert | *Return Address Label Small | 224 × 1200 / 194 × 1140 / 15 × 30 |
| `LeitzShippingLabel70130001` | — | — | `LeitzShippingLabel70130001` | nicht gesichert | *Shipping Label 2.33"x4" | 697 × 2410 / 667 × 2350 / 15 × 30 |
| `LeitzShippingLabel70180001` | — | — | `LeitzShippingLabel70180001` | nicht gesichert | *Shipping Label 2"x3.5" | 1039 × 1182 / 960 × 1122 / 40 × 30 |
| `LeitzShippingLabel70180001Rotated` | — | — | `LeitzShippingLabel70180001Rotated` | nicht gesichert | *Shipping Label 2"x3.5" R | 591 × 2078 / 561 × 2018 / 15 × 30 |
| `LeitzStandardAddressLabel` | — | — | `LeitzStandardAddressLabel` | nicht gesichert | *Standard Address Label | 1039 × 662 / 960 × 602 / 40 × 30 |
| `LeitzSuspensionFileLabel` | — | — | `LeitzSuspensionFileLabel` | nicht gesichert | *Suspension File Label | 148 × 1200 / 118 × 1140 / 15 × 30 |
| `LeitzWideLeverArchCardShort` | — | — | `LeitzWideLeverArchCardShort` | nicht gesichert | *Wide Lever Arch Card Short | 673 × 4488 / 643 × 4428 / 15 × 30 |
| `LeitzWideLeverArchLabelLong` | — | — | `LeitzWideLeverArchLabelLong` | nicht gesichert | *Wide Lever Arch Label Long | 720 × 6732 / 690 × 6702 / 15 × 30 |
| `LeitzWideLeverArchLabelShort` | — | — | `LeitzWideLeverArchLabelShort` | nicht gesichert | *Wide Lever Arch Label Short | 720 × 4536 / 690 × 4476 / 15 × 30 |
| `LeitzWristband` | — | — | — | nicht gesichert | *Wristband | — |
| `LeverArchFile1005` | — | — | — | nicht gesichert | Active File | — |
| `Multipurpose70030001` | Multipurpose 2" x 3 1/2" | Multipurpose 2" x 3 1/2" | `Multipurpose70030001` | nicht gesichert | Multipurpose 2" x 3 1/2" | 1039 × 1200 / 960 × 1140 / 40 × 30 |
| `Multipurpose70030001Rotated` | Multipurpose 2" x 3 1/2" Rotated | Multipurpose 2" x 3 1/2" Rotated | — | nicht gesichert | Multipurpose 2" x 3 1/2" Rotated | — |
| `Multipurpose70090001` | Multipurpose 1" x 1 1/2" | Multipurpose 1" x 1 1/2" | `Multipurpose70090001` | nicht gesichert | Multipurpose 1" x 1 1/2" | 295 × 900 / 265 × 840 / 15 × 30 |
| `Multipurpose70100001` | Multipurpose 3/4" x 2 1/2" | Multipurpose 3/4" x 2 1/2" | `Multipurpose70100001` | nicht gesichert | Multipurpose 3/4" x 2 1/2" | 224 × 1500 / 194 × 1440 / 15 × 30 |
| `Multipurpose70110001` | Multipurpose 1/2" x 1 7/8" | Multipurpose 1/2" x 1 7/8" | `Multipurpose70110001` | nicht gesichert | Multipurpose 1/2" x 1 7/8" | 148 × 1134 / 118 × 1074 / 15 × 30 |
| `NameBadgeCard70190001` | Name Badge Card | Name Badge Card | `NameBadgeCard70190001` | nicht gesichert | Name Badge Card | 1075 × 1350 / 960 × 1290 / 58 × 30 |
| `NameBadgeCard70190001Rotated` | Name Badge Rotated | Name Badge Rotated | — | nicht gesichert | Name Badge Rotated | — |
| `NameBadgeCard8214` | Durable Label | Durable Label | `NameBadgeCard8214` | nicht gesichert | Durable Label | 1075 × 1276 / 960 × 1216 / 58 × 30 |
| `NameBadgeLabel70030001` | Name Badge | Name Badge | `NameBadgeLabel70030001` | nicht gesichert | Name Badge | 1039 × 1398 / 960 × 1338 / 40 × 30 |
| `NameBadgeLabel70060001` | Name Badge Label | Name Badge Label | `NameBadgeLabel70060001` | nicht gesichert | Name Badge Label | 591 × 1800 / 561 × 1740 / 15 × 30 |
| `NarrowLeverArchCardShort` | Narrow Lever Arch Card - Short | Narrow Lever Arch Card - Short | `NarrowLeverArchCardShort` | nicht gesichert | Narrow Lever Arch Card - Short | 378 × 4488 / 348 × 4428 / 15 × 30 |
| `NarrowLeverArchLabelLong` | Narrow Lever Arch Label, Long | Narrow Lever Arch Label, Long | `NarrowLeverArchLabelLong` | nicht gesichert | Narrow Lever Arch Label - Long | 461 × 6732 / 431 × 6672 / 15 × 30 |
| `NarrowLeverArchLabelShort` | Narrow Lever Arch Label, Short | Narrow Lever Arch Label, Short | `NarrowLeverArchLabelShort` | nicht gesichert | Narrow Lever Arch Label - Short | 461 × 4536 / 431 × 4476 / 15 × 30 |
| `PackageShippingLabel` | Package Label | Package Label | `PackageShippingLabel` | nicht gesichert | Package Label | 1039 × 3600 / 960 × 3540 / 40 × 30 |
| `ReturnAddressLabelLarge` | Return Address Label, Large | Return Address Label, Large | `ReturnAddressLabelLarge` | nicht gesichert | Return Address Label - Large | 295 × 1276 / 265 × 1216 / 15 × 30 |
| `ReturnAddressLabelSmall` | Return Address Label, Small | Return Address Label, Small | `ReturnAddressLabelSmall` | nicht gesichert | Return Address Label - Small | 224 × 1200 / 194 × 1140 / 15 × 30 |
| `ShippingLabel70130001` | Shipping Label 2 1/3" x 4" | Shipping Label 2 1/3" x 4" | `ShippingLabel70130001` | nicht gesichert | Shipping Label 2 1/3" x 4" | 697 × 2410 / 667 × 2350 / 15 × 30 |
| `ShippingLabel70180001` | Shipping Label 2" x 3 1/2" | Shipping Label 2" x 3 1/2" | `ShippingLabel70180001` | nicht gesichert | Shipping Label 2" x 3 1/2" | 1039 × 1182 / 960 × 1122 / 40 × 30 |
| `ShippingLabel70180001Rotated` | Shipping Label 2" x 3 1/2" Rotated | Shipping Label 2" x 3 1/2" Rotated | — | nicht gesichert | Shipping Label 2" x 3 1/2" Rotated | — |
| `StandardAddressLabel` | Standard Address Label | Standard Address Label | `StandardAddressLabel` | nicht gesichert | Standard Address Label | 1039 × 662 / 960 × 602 / 40 × 30 |
| `SuspensionFileLabel` | Suspension File Label | Suspension File Label | `SuspensionFileLabel` | nicht gesichert | Suspension File Label | 148 × 1200 / 118 × 1140 / 15 × 30 |
| `WideLeverArchCardShort` | Wide Lever Arch Card - Short | Wide Lever Arch Card - Short | `WideLeverArchCardShort` | nicht gesichert | Wide Lever Arch Card - Short | 673 × 4488 / 643 × 4428 / 15 × 30 |
| `WideLeverArchLabelLong` | Wide Lever Arch Label - Long | Wide Lever Arch Label - Long | `WideLeverArchLabelLong` | nicht gesichert | Wide Lever Arch Label - Long | 720 × 6732 / 690 × 6702 / 15 × 30 |
| `WideLeverArchLabelShort` | Wide Lever Arch Label - Short | Wide Lever Arch Label - Short | `WideLeverArchLabelShort` | nicht gesichert | Wide Lever Arch Label - Short | 720 × 4536 / 690 × 4476 / 15 × 30 |
| `Wristband` | — | — | `Wristband` | nicht gesichert | Wristband | 336 × 6554 / 306 × 6494 / 15 × 30 |

## Zehn Entwicklungsbezeichner ohne Eintrag in dieser Windows-GPD

- `Continuous112`
- `LargePendaflexTabRotated`
- `LeitzContinuous112`
- `LeitzDHLShippingLabel`
- `LeitzNameBadgeCard8214`
- `LeitzWristband`
- `LeverArchFile1005`
- `Multipurpose70030001Rotated`
- `NameBadgeCard70190001Rotated`
- `ShippingLabel70180001Rotated`

Die fehlende GPD-Option sagt **nichts** über die mechanische Möglichkeit des Druckers aus. Einige Optionen sind Linux-/macOS-spezifische Varianten oder Aliase. Sie werden nicht stillschweigend auf andere Windows-Namen abgebildet.

## Nachweisgrenzen und offener nächster Schritt

- Aus der ausgelesenen Original-GPD verifiziert: exakt 77 verschiedene `*Option`-Namen, 76 fest und `CUSTOMSIZE`; Originalnamen zu allen 77 ermittelt.
- Die nur im Entwicklungsmodell hinterlegten zehn Bezeichner sind keine Windows-GPD-Einträge dieses Originals.
- Numerische `OptionID` und `rcNameID` sowie die Befehlsbytes werden hier nicht aus der Entwicklungsgeometrie **abgeleitet**. Nur `DHLShippingLabel`/335 und `CUSTOMSIZE` sind bereits separat als Befund belegt.
- Der noch ausstehende vollständige Windows-Datensatzauszug mit allen originalen OptionIDs, Command-Bytes und Ressourcenkennungen benötigt den wieder zugänglichen Originalinhalt. Dafür ist bereits `compare_windows_gpd_four_sources.py` vorhanden.
- Keine Datei des Originalherstellers wurde ausgeführt; keine Gerätekommunikation, Druckertreiberinstallation oder CUPS-Änderung.
