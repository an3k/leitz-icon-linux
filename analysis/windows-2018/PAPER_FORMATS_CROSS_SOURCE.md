# Papierformatvergleich – nachgewiesener Quellenstand

Die vier Quellen werden **getrennt** geführt; Übereinstimmung eines internen Namens beweist keine physische Kassettenidentität. `—` in der Windows-Spalte bedeutet **nicht verifiziert**, nicht „im Original nicht vorhanden“.

| Quelle | Belegter Umfang |
| --- | --- |
| Linux 1.0.3 | **40** `PageSize`-Einträge aus `sources` (`f8452657`), `originals/linux-1.0.3/LeitzIcon.ppd` |
| macOS 5.1.465 Classic | **41** `PageSize`-Einträge aus `sources` (`f8452657`), `originals/macos-5.1.465-classic/LeitzIcon.ppd` |
| Windows 2018 | **77** dokumentierte GPD-Optionen (**76 fest + `CUSTOMSIZE`**); Originale Windows-GPD SHA-256 `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`; derzeit nur die **bereits dokumentierten** zwei Bezeichner, nicht der vollständige GPD-Inhalt. **75 weitere Windows-Namen sind hier nicht inventarisiert.** |
| Linux-Eigenentwicklung | **86** Medienprofile aus `driver` (`3289f1bd`), `media/media.json` |

## Vergleich nach exakt übereinstimmenden internen Bezeichnern

Es werden Original-Labels unverändert wiedergegeben. Windows: Nur `DHLShippingLabel` und `CUSTOMSIZE` sind aus den gesicherten Originalbefunden namentlich bestätigt. Alle übrigen Windows-Formatnamen bleiben unbekannt; daher erfolgen keine vermuteten Zuordnungen.

| Interner Bezeichner | Original Linux (lesbarer Name) | Original macOS (lesbarer Name) | Original Windows (belegter Bezeichner) | Linux-Eigenentwicklung (lesbarer Name) |
| --- | --- | --- | --- | --- |
| `AppointmentCard` | Appointment Card | Appointment Card | — | Appointment Card |
| `CUSTOMSIZE` | — | — | `CUSTOMSIZE` | — |
| `Continuous112` | Continuous - 1 1/2" | Continuous - 1 1/2" | — | Continuous - 1 1/2" |
| `Continuous70030001` | Continuous - 3 1/2" | Continuous - 3 1/2" | — | Continuous - 3 1/2" |
| `Continuous70040001` | Continuous - 2 2/5" | Continuous - 2 2/5" | — | Continuous - 2 2/5" |
| `Continuous70050001` | Continuous - 2 1/5" | Continuous - 2 1/4" | — | Continuous - 2 1/4" |
| `Continuous70060001` | Continuous - 2" | Continuous - 2" | — | Continuous - 2" |
| `Continuous70070001` | — | — | — | Continuous - 1 1/2" |
| `Continuous70080001` | Continuous - 1 1/4" | Continuous - 1 1/4" | — | Continuous - 1 1/4" |
| `Continuous70090001` | Continuous - 1" | Continuous - 1" | — | Continuous - 1" |
| `Continuous70100001` | Continuous - 3/4" | Continuous - 3/4" | — | Continuous - 3/4" |
| `Continuous70110001` | Continuous - 1/2" | Continuous - 1/2" | — | Continuous - 1/2" |
| `Continuous70190001` | Continuous - 3 3/5" | Continuous - 3 3/5" | — | Continuous - 3 3/5" |
| `DHLShippingLabel` | — | DHLShippingLabel | `DHLShippingLabel` | DHL Shipping Label |
| `FileFolderLabel` | File Folder Label | File Folder Label | — | File Folder Label |
| `InternationalAddressLabel` | International Address Label | International Address Label | — | International Address Label |
| `LargePendaflexTab` | Large Pendaflex Tab | Large Pendaflex Tab | — | Large Pendaflex Tab |
| `LargePendaflexTabRotated` | Large Pendaflex Tab Rotated | Large Pendaflex Tab Rotated | — | Large Pendaflex Tab Rotated |
| `LeitzAppointmentCard` | — | — | — | *Appointment Card |
| `LeitzContinuous112` | — | — | — | *Continuous - 1 1/2" |
| `LeitzContinuous70030001` | — | — | — | *Continuous - 3.5" |
| `LeitzContinuous70040001` | — | — | — | *Continuous - 2.4" |
| `LeitzContinuous70050001` | — | — | — | *Continuous - 2.25" |
| `LeitzContinuous70060001` | — | — | — | *Continuous - 2" |
| `LeitzContinuous70070001` | — | — | — | *Continuous - 1.54" |
| `LeitzContinuous70080001` | — | — | — | *Continuous - 1.25" |
| `LeitzContinuous70090001` | — | — | — | *Continuous - 1" |
| `LeitzContinuous70100001` | — | — | — | *Continuous - 0.75" |
| `LeitzContinuous70110001` | — | — | — | *Continuous - 0.5" |
| `LeitzContinuous70190001` | — | — | — | *Continuous - 3.6" |
| `LeitzDHLShippingLabel` | — | — | — | *DHL Shipping Label |
| `LeitzFileFolderLabel` | — | — | — | *File Folder Label |
| `LeitzInternationalAddressLabel` | — | — | — | *Large Address Label |
| `LeitzLargePendaflexTab` | — | — | — | *Large Pendaflex Tab |
| `LeitzLargePendaflexTabRotated` | — | — | — | *Large Pendaflex Tab R |
| `LeitzLeverArchFile1005` | Active File | Active File | — | *Lever Arch File 1005 |
| `LeitzMultipurpose70030001` | — | — | — | *Multipurpose 2"x3.5" |
| `LeitzMultipurpose70030001Rotated` | — | — | — | *Multipurpose 2"x3.5" R |
| `LeitzMultipurpose70090001` | — | — | — | *Multipurpose 1"x1.5" |
| `LeitzMultipurpose70100001` | — | — | — | *Multipurpose 0.75"x2.5" |
| `LeitzMultipurpose70110001` | — | — | — | *Multipurpose 0.5"x1.875" |
| `LeitzNameBadgeCard70190001` | — | — | — | *Name Badge Card 2.25"x3.5" |
| `LeitzNameBadgeCard70190001Rotated` | — | — | — | *Name Badge Card 2.25"x3.5" R |
| `LeitzNameBadgeCard8214` | — | — | — | *Durable Label |
| `LeitzNameBadgeLabel70030001` | — | — | — | *Name Badge 2.33"x3.5" |
| `LeitzNameBadgeLabel70060001` | — | — | — | *Name Badge Label 2"x3" |
| `LeitzNarrowLeverArchCardShort` | — | — | — | *Narrow Lever Arch Card Short |
| `LeitzNarrowLeverArchLabelLong` | — | — | — | *Narrow Lever Arch Label Long |
| `LeitzNarrowLeverArchLabelShort` | — | — | — | *Narrow Lever Arch Label Short |
| `LeitzPackageShippingLabel` | — | — | — | *Package Label 6"x3.5" |
| `LeitzReturnAddressLabelLarge` | — | — | — | *Return Address Label Large |
| `LeitzReturnAddressLabelSmall` | — | — | — | *Return Address Label Small |
| `LeitzShippingLabel70130001` | — | — | — | *Shipping Label 2.33"x4" |
| `LeitzShippingLabel70180001` | — | — | — | *Shipping Label 2"x3.5" |
| `LeitzShippingLabel70180001Rotated` | — | — | — | *Shipping Label 2"x3.5" R |
| `LeitzStandardAddressLabel` | — | — | — | *Standard Address Label |
| `LeitzSuspensionFileLabel` | — | — | — | *Suspension File Label |
| `LeitzWideLeverArchCardShort` | — | — | — | *Wide Lever Arch Card Short |
| `LeitzWideLeverArchLabelLong` | — | — | — | *Wide Lever Arch Label Long |
| `LeitzWideLeverArchLabelShort` | — | — | — | *Wide Lever Arch Label Short |
| `LeitzWristband` | — | — | — | *Wristband |
| `LeverArchFile1005` | — | — | — | Active File |
| `Multipurpose70030001` | Multipurpose 2" x 3 1/2" | Multipurpose 2" x 3 1/2" | — | Multipurpose 2" x 3 1/2" |
| `Multipurpose70030001Rotated` | Multipurpose 2" x 3 1/2" Rotated | Multipurpose 2" x 3 1/2" Rotated | — | Multipurpose 2" x 3 1/2" Rotated |
| `Multipurpose70090001` | Multipurpose 1" x 1 1/2" | Multipurpose 1" x 1 1/2" | — | Multipurpose 1" x 1 1/2" |
| `Multipurpose70100001` | Multipurpose 3/4" x 2 1/2" | Multipurpose 3/4" x 2 1/2" | — | Multipurpose 3/4" x 2 1/2" |
| `Multipurpose70110001` | Multipurpose 1/2" x 1 7/8" | Multipurpose 1/2" x 1 7/8" | — | Multipurpose 1/2" x 1 7/8" |
| `NameBadgeCard70190001` | Name Badge Card | Name Badge Card | — | Name Badge Card |
| `NameBadgeCard70190001Rotated` | Name Badge Rotated | Name Badge Rotated | — | Name Badge Rotated |
| `NameBadgeCard8214` | Durable Label | Durable Label | — | Durable Label |
| `NameBadgeLabel70030001` | Name Badge | Name Badge | — | Name Badge |
| `NameBadgeLabel70060001` | Name Badge Label | Name Badge Label | — | Name Badge Label |
| `NarrowLeverArchCardShort` | Narrow Lever Arch Card - Short | Narrow Lever Arch Card - Short | — | Narrow Lever Arch Card - Short |
| `NarrowLeverArchLabelLong` | Narrow Lever Arch Label, Long | Narrow Lever Arch Label, Long | — | Narrow Lever Arch Label - Long |
| `NarrowLeverArchLabelShort` | Narrow Lever Arch Label, Short | Narrow Lever Arch Label, Short | — | Narrow Lever Arch Label - Short |
| `PackageShippingLabel` | Package Label | Package Label | — | Package Label |
| `ReturnAddressLabelLarge` | Return Address Label, Large | Return Address Label, Large | — | Return Address Label - Large |
| `ReturnAddressLabelSmall` | Return Address Label, Small | Return Address Label, Small | — | Return Address Label - Small |
| `ShippingLabel70130001` | Shipping Label 2 1/3" x 4" | Shipping Label 2 1/3" x 4" | — | Shipping Label 2 1/3" x 4" |
| `ShippingLabel70180001` | Shipping Label 2" x 3 1/2" | Shipping Label 2" x 3 1/2" | — | Shipping Label 2" x 3 1/2" |
| `ShippingLabel70180001Rotated` | Shipping Label 2" x 3 1/2" Rotated | Shipping Label 2" x 3 1/2" Rotated | — | Shipping Label 2" x 3 1/2" Rotated |
| `StandardAddressLabel` | Standard Address Label | Standard Address Label | — | Standard Address Label |
| `SuspensionFileLabel` | Suspension File Label | Suspension File Label | — | Suspension File Label |
| `WideLeverArchCardShort` | Wide Lever Arch Card - Short | Wide Lever Arch Card - Short | — | Wide Lever Arch Card - Short |
| `WideLeverArchLabelLong` | Wide Lever Arch Label - Long | Wide Lever Arch Label - Long | — | Wide Lever Arch Label - Long |
| `WideLeverArchLabelShort` | Wide Lever Arch Label - Short | Wide Lever Arch Label - Short | — | Wide Lever Arch Label - Short |
| `Wristband` | — | — | — | Wristband |

## Identitäts- und Quellenkonflikte

- Die 40 Linux-Bezeichner finden sich sämtlich auch in macOS Classic und im Entwicklungskatalog. macOS enthält gegenüber Linux zusätzlich genau `DHLShippingLabel`.
- Der Entwicklungskatalog besitzt **45** Bezeichner, die in keiner der beiden Original-PPDs stehen. Darunter sind Leitz-Präfixvarianten, deren eigenständige physische Bedeutung **nicht** aus ihrem Namen abgeleitet wird.
- `Continuous70050001` heißt im Linux-Original **Continuous - 2 1/5"**, im macOS-Original **Continuous - 2 1/4"**. Diese Bezeichnungen werden nicht vereinheitlicht.
- `DHLShippingLabel` besitzt in der belegten Windows-GPD die `OptionID=335`, `PageDimensions=(1039,4204)`, `PrintableArea=(960,4144)`, `PrintableOrigin=(40,30)` und die Befehlsbytes `1B 4C 08 36 1B 77 03 C0`. Im Entwicklungskatalog steht dagegen irrtümlich `"not present in Windows GPD"` in `source`; der Katalog wurde hier **nicht** verändert.
- `CUSTOMSIZE` ist ein Windows-Format ohne feste Größe. Es wird nicht mit den plattformspezifischen `CustomPageSize`-Mechanismen gleichgesetzt, solange deren Zuordnung nicht bewiesen ist.
- Die PPD-Papierabmessungen, die tatsächliche Druckfläche, die Kassettenbreite und die mechanische Schnittfunktion bleiben getrennte Angaben.

## Nachweislücke

Die ursprüngliche Windows-GPD war bei dieser Überprüfung nicht als vollständige Datei verfügbar. Eine Auflistung der **75 nicht namentlich gesicherten Windows-Optionen** oder deren Gleichsetzung mit Linux/macOS wäre deshalb unbelegt. Für einen vollständigen Windows-Vergleich muss die originale `LeitzIcon.gpd` mit dem angegebenen SHA-256-Hash analysiert werden. Das bereits vorhandene Programm `analysis/windows-2018/inventory_windows_gpd.py` kann diese Datei aus dem Originalarchiv auslesen, sobald dessen Binärinhalt bereitsteht. Die vorhandene 77er-Zählung ist nicht mit einer vollständigen zeilenweisen Zuordnung gleichzusetzen.

Erstellt ausschließlich als Quellenvergleich; keine Änderung des produktiven Treibers und keine Gerätekommunikation.
