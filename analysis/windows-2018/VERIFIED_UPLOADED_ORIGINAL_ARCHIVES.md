# Direkt geprüfte Originalarchive und bisher offene Windows-Funktionen

Alle drei am 17.10.2026 erneut hochgeladenen Archive wurden **direkt aus
den ZIP-Bytes** mit Python `zipfile` gelesen, ohne Installer oder
herstellereigenen Binärcode auszuführen.

| Datei | Tatsächlich geprüfte ZIP-SHA-256 |
| --- | --- |
| LeitzIconDriversForLinux1.0.3(4).zip | `54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17` |
| LeitzIconLabelSetup(5).zip | `cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca` |
| LeitzIconSoftware-Installer_5.1.465.dmg(2).zip | `6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210` |

Auszug und Gegentest:
- Original Linux `raster2printer.c` (18.683 Bytes),
  SHA-256 `bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0`,
  identisch mit `originals/linux-1.0.3`.
- Original Windows `LeitzIcon.gpd` (30.336 Bytes),
  SHA-256 `f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7`,
  identisch mit zuvor verifiziertem Original, **nicht** mit der
  zeilenendennormalisierten Texttranskription gleichgesetzt.
- Original `PrintHelper.dll` (18.944 Bytes),
  SHA-256 `0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a`.
- macOS Classic `raster2printer` (84.252 Bytes),
  SHA-256 `72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13`.
  Zusätzlich enthalten: `command2leitz` und V3-Varianten.
- Windows x86/x64 `LeitzDevMode.dll`, `LeitzUSRes.dll`,
  `LeitzUI.dll`, `LeitzRp.dll` und `LeitzLm.dll` aus
  der Originalstruktur separat extrahiert; kein Binärcode gestartet.

## Echte Windows-Anzeigetexte rekonstruiert

Aus den Originalen `Drivers/amd64/LeitzUSRes.dll` und
`Drivers/i386/LeitzUSRes.dll` wurden alle **77 RT_STRING-Texte der
Sprache 0x0409 (English US)** direkt aus der PE-Ressourcenstruktur
ausgelesen. Beide Architekturen enthalten dieselben 77 Strings für
Ressourcen-ID 259 bis 335. SHA-256:
- amd64: `bdb4de0b39750c9391f6626bbc487977b5f6e60c1c2ec357fc2877b90b0cb4c4`
- i386: `0d7c5064b373490a8db1649a0cb74d9dd46691dd84ac4aa8cd3caae7362494b0`.

**Alle 76 festen GPD-Papierformate** haben exakt eine numerische
`rcNameID`-Zuordnung zu diesen Windows-Originalnamen. Zu beachten:
`DHLShippingLabel` ist im Original als
`*Package Label 7"x3.5"` sichtbar; `NameBadgeCard8214` als
`Name Badge Card - Durable© 8214`. Sterne, Interpunktion,
Dezimalangaben und Sonderzeichen wurden nicht normalisiert.

**Wichtig:** Die separate String-ID 333 enthält zwar
`*Custom paper size`, aber der GPD-Block `CUSTOMSIZE` referenziert
`=USER_DEFINED_SIZE_DISPLAY` aus `StdNames.gpd` und nicht String
333. Deshalb wird kein direkter OEM-Name für CUSTOMSIZE erfunden.

Die Datei `windows_2018_original_ui_strings.json` enthält alle 76
zugeordneten Originalnamen sowie den separat unbelegten String 333;
`FOUR_SOURCE_WITH_WINDOWS_UI_NAMES.md` ergänzt den vollständigen
vierfachen Medienvergleich um die originale Windows-Anzeigespalte.
`gpd_ui_resources.[ch]` und seine 77er C-Tabelle stellen diese
originalen Texte ohne OEM DLL-Abhängigkeit bereit. Der C-Generator
verifiziert die Mapping-Schlüssel anhand des originalen 77er Inventars.

## Fortschritt und Grenzen

Der separate Original-Windows-Baustein `LeitzDevMode.dll/hrOEMDevMode`
ist jetzt für x86/x64 aus Originaldisassemblierung in portables C
übertragen. Unveränderte Windows-DLL-ABI, weitere Funktionen der
`LeitzUI.dll`, COM/Unidrv-Laufzeitsequenz, tatsächliche Statusframes
und Geräte-Druck-/Schnittoperationen bleiben offen.

Alle Dateien betreffen allein `sources`; die produktiven Branches
und das tatsächliche Gerät wurden nicht geändert.
