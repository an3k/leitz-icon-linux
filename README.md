# Leitz Icon USB — Linux / Unraid / CUPS

**Entwicklung, nicht freigegeben.** Maßgeblicher Branch: `port/vendor-parity`.
`main` enthält einen älteren experimentellen Vergleichsstand. Kein Release
und keine Hardwarefreigabe der neu modularisierten Version.

## Architektur

Der produktive Treiber liegt ausschließlich in `driver/`, die unabhängigen
Tests unter `tests/`. Kein proprietärer Originaltreiber wird mitgeliefert.

| Datei | Verantwortung |
| --- | --- |
| `driver/src/leitz_raster.c` | Gemeinsame CUPS-Raster- und Medienverarbeitung |
| `driver/src/leitz_jobs.c/.h` | Mehrseitenauftrag, Breite/Auflösung, gemeinsame Schnittentscheidung |
| `driver/src/leitz_protocols.c/.h` | ABI-geprüfte, dynamische Protokollauswahl |
| `driver/src/leitz_protocol_current.c` | Eigenständiger Current-Encoder und Validator |
| `driver/src/leitz_protocol_native.c` | Eigenständiger Native-Encoder und Validator |
| `driver/cups/leitzusb` | Spoolvalidierung, Kassettenprüfung und USB-Übertragung |
| `driver/media/media.json` | Ausschließlich maßgebliche MASTER-Liste der 86 Medienprofile |
| `driver/build/generate_media.py` | Erzeugt PPD-Medienoptionen und C-Header ohne separate Namenslisten |

Bei `make all` entstehen `driver/modules/leitz-current.so` und
`driver/modules/leitz-native.so`. Die CUPS-Filterprogramme enthalten keine
statisch eingebundenen Encoder. Im Container werden die Module unter
`/usr/lib/leitz/protocols/` installiert. Zum Laden kann `LEITZ_PROTOCOL_DIR`
ein anderes Modulverzeichnis vorgeben.

**Die installierte PPD bleibt beim Entfernen oder Hinzufügen eines Moduls unverändert.**
`LeitzProtocol=Auto` ist Standard: zuerst `Current`, sonst `Native`.
`LeitzProtocol=Current` und `LeitzProtocol=Native` sind explizite Einstellungen:
wenn das verlangte Modul fehlt oder inkompatibel ist, wird der Auftrag abgewiesen.
Die jeweilige Auswahl erfolgt bei einem neuen CUPS-Filterprozess. Ein eigenes
Verzeichnisüberwachungs- oder Modulverwaltungsprogramm existiert nicht.
Neue Protokolle erfordern eine neue Treiberversion samt PPD-Option.

Der generische USB-Backend-Validator probiert die installierten Protokollmodule
am vollständigen Spool aus, bevor USB geöffnet wird. Kein automatischer
Neudruck bei unklarem Übertragungszustand.

## Papierformate

**86 verbindliche Profile: 43 A + 43 B.** Lesbare CUPS-Namen, interne IDs,
Quellgeometrien, Druckbereiche und Längencodes sind in der MASTER-Liste
gespeichert. Vier voneinander abweichende A/B-Geometrien bleiben ausdrücklich
erhalten. `driver/src/leitz_media.h` und `driver/cups/LeitzIcon.ppd`
werden ausschließlich daraus generiert. Die verwendeten 86 Namen wurden
aus dem bereits vorhandenen Entwicklungsbestand übernommen, nicht neu benannt.
Eine separate Pflege der historischen Linux-/macOS-/Windows-Listen findet
nicht statt.

## Build und Offline-Tests

Für den Build werden GCC, make, libcups2-dev und Python 3 benötigt:

```sh
make clean all cups mock-filter probe ppd
make media-check native-reference-test job-geometry-test protocol-isolation-test status-test probe-test
LEITZ_PROTOCOL_DIR="$PWD/driver/modules" python3 tests/integration/test_core.py
LEITZ_PROTOCOL_DIR="$PWD/driver/modules" python3 tests/integration/test_v04.py
```

Der Integrationstest `test_protocol_isolation.py` entfernt und ergänzt
in einem temporären Modulverzeichnis die kompilierten `.so`-Dateien und
prüft `Auto`, die manuelle Auswahl und die unveränderte PPD. Die anderen
Offline-Tests kontrollieren Medien, Befehlsrahmen, Raster, Schnittoptionen,
Mehrseitendruck und Statusformatierung. GitHub Actions prüft zudem CUPS-PPD
und baut ein eigenständiges Docker-Image. **Das ersetzt keinen Gerätetest.**

## Hardwarestand und bekannte Grenzen

Am älteren v0.4.1-Stand wurden mit weißem 88-mm-Endlosmaterial
300×300 und 300×600 dpi, unterschiedliche Längen, Mehrseitendruck sowie
automatisches Schneiden nachgewiesen. `ESC U` wurde an realen Kassetten
abgefragt; `ESC A` lieferte `0x03` im Leerlauf.

Die aktuelle dynamische Integration und `Native` haben **noch keinen
erfolgreichen Gerätetest**. Bei gelbem Material traten zuvor eine
Längenabweichung und ein abgebrochener Druck auf, ohne gesicherte Ursache.
USB-Schreiberfolg oder CUPS `idle` beweist noch keinen mechanisch
abgeschlossenen Druck. Physische Medien-/Statusabdeckung und
Wiederherstellung müssen vor einer Freigabe validiert werden.

Weitere Details: [Projektstatus](docs/PROJECT_STATUS.md),
[Portierungsanalyse](docs/PORTING_STATUS.md),
[Unraid](docs/UNRAID.md).

**Lizenz:** Noch keine Veröffentlichungslizenz gewählt.
