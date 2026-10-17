#!/usr/bin/env python3
"""Offline four-source inventory of original Windows GPD and PPD/media data.

No physical cassette assumptions, no Windows UI string invention. The Windows
GPD has rcNameID references, not the actual translated UI resource strings.
Requires the SHA-256-verified original manufacturer's GPD in its installer ZIP.
"""
import argparse
import hashlib
import json
import re
import sys
import zipfile
from pathlib import Path

from inventory_windows_gpd import paper_options

EXPECTED_GPD_SHA256 = "f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7"
GPD_ZIP_PATH = "LeitzIconLabelSetup/SourceDir/Leitz Icon Software/Drivers/LeitzIcon.gpd"
BASE = Path(__file__).resolve().parents[2]
LINUX = BASE / "originals/linux-1.0.3/LeitzIcon.ppd"
MACOS = BASE / "originals/macos-5.1.465-classic/LeitzIcon.ppd"
PAGE_LINE = re.compile(r"^\*PageSize[ \t]+([A-Za-z0-9_]+)\/([^:\r\n]*):")


def read_ppd(path):
    """Read exact original PageSize internal IDs and labels (not geometry)."""
    results = {}
    for line in Path(path).read_text(encoding="latin-1").splitlines():
        found = PAGE_LINE.match(line)
        if found:
            identifier, label = found.groups()
            if identifier in results:
                raise ValueError(f"Duplicate PageSize: {identifier!r}")
            results[identifier] = label
    return results


def read_development(path):
    result = {}
    items = json.loads(Path(path).read_text(encoding="utf-8"))
    if not isinstance(items, list):
        raise ValueError("Development media catalog is not an array")
    for entry in items:
        name = entry["name"]
        if name in result:
            raise ValueError(f"Duplicate development ID: {name!r}")
        result[name] = entry
    return result


def load_windows_original(zip_path):
    with zipfile.ZipFile(zip_path) as archive:
        payload = archive.read(GPD_ZIP_PATH)
    found_hash = hashlib.sha256(payload).hexdigest()
    if found_hash != EXPECTED_GPD_SHA256:
        raise ValueError(f"Wrong Windows GPD SHA-256: {found_hash}; expected {EXPECTED_GPD_SHA256}")
    options = paper_options(payload.decode("ascii"))
    if len(options) != 77 or len({o["name"] for o in options}) != 77:
        raise ValueError("Expected 77 distinct Windows GPD PaperSize options")
    fixed = [o for o in options if o["page_dimensions"] is not None]
    custom = [o for o in options if o["page_dimensions"] is None]
    if len(fixed) != 76 or len(custom) != 1 or custom[0]["name"] != "CUSTOMSIZE":
        raise ValueError("Expected 76 fixed paper definitions and CUSTOMSIZE")
    ids = [o["option_id"] for o in fixed]
    if any(i is None for i in ids) or len(set(ids)) != 76:
        raise ValueError("Fixed Windows OptionID values missing or duplicated")
    dhl = next((o for o in options if o["name"] == "DHLShippingLabel"), None)
    if (dhl is None or dhl["option_id"] != 335 or
            dhl["page_dimensions"] != [1039, 4204] or
            dhl["printable_area"] != [960, 4144] or
            dhl["printable_origin"] != [40, 30] or
            dhl["command_bytes_hex"] != "1b4c08361b7703c0"):
        raise ValueError("Known authentic DHLShippingLabel entry mismatch")
    return options


def geometry_match(win, dev):
    """Original 300x600 GPD units vs development dimensions, same ID only."""
    if win is None or dev is None:
        return "—"
    if win["page_dimensions"] is None:
        return "variabel (nicht vergleichbar)"
    dimensions = [("page_dimensions", "page"),
                  ("printable_area", "area"),
                  ("printable_origin", "origin")]
    differences = [g for g, d in dimensions
                   if win.get(g) is None or win[g] != dev.get(d)]
    return "identische Geometriewerte" if not differences else "abweichend: " + ", ".join(differences)


def cell(text):
    if text is None:
        return "—"
    return str(text).replace("|", "\\|").replace("\n", " ").replace("\r", " ")


def render(options, linux, macos, development):
    win = {o["name"]: o for o in options}
    if len(win) != len(options):
        raise ValueError("Duplicate Windows paper names")
    ids = sorted(set(linux) | set(macos) | set(win) | set(development))
    lines = [
        "# Original-Windows-GPD: Vier-Quellen-Papierinventur",
        "",
        "**Quellen getrennt:** Original Linux 1.0.3, Original macOS Classic 5.1.465, "
        "Original Windows 2018 GPD, Linux-Eigenentwicklung (media.json).",
        "",
        f"**Anzahlen:** Linux {len(linux)}; macOS {len(macos)}; "
        f"Windows {len(win)}; Eigenentwicklung {len(development)}; "
        f"vereinigte exakte IDs {len(ids)}.",
        "",
        "**Windows-Namen:** unveränderte interne GPD-Optionennamen. "
        "Die Windows-UI-Klartextnamen sind ohne die rcNameID-Ressourcen "
        "nicht verfügbar und werden nicht erfunden.",
        "",
        "**Geometrie:** Windows-GPD- und Entwicklungswerte werden ausschließlich "
        "bei exakt gleichem Namen verglichen, in MasterUnits (300, 600). "
        "Dies beweist keine identische Kassette oder reale bedruckbare Fläche.",
        "",
        "| Interne ID | Linux Originalname | macOS Originalname | "
        "Windows Original-ID | Windows rcNameID | Windows OptionID | "
        "Entwicklungsname | Windows/Entwicklung Geometrie |",
        "| --- | --- | --- | --- | --- | --- | --- | --- |",
    ]
    for name in ids:
        w, d = win.get(name), development.get(name)
        lines.append("| " + " | ".join([
            f"`{name}`", cell(linux.get(name)), cell(macos.get(name)),
            f"`{w['name']}`" if w else "—",
            cell(w["resource_name_id"] if w else None),
            cell(w["option_id"] if w else None),
            cell(d["label"] if d else None), cell(geometry_match(w, d))
        ]) + " |")
    lines += ["", "## Original-Windows-Befehlsbytes (nicht durch GPD erweitert)", "",
              "Diese Befehlsbytes sind die originalen PaperSize-Auswahlfolgen, "
              "nicht ein kompletter Druckauftrag. Kein Geräte-I/O.", "",
              "| Windows-Option | OptionID | Befehl hex | Seitenmaß | Druckfläche | Ursprung |",
              "| --- | --- | --- | --- | --- | --- |"]
    for w in options:
        def pair(val):
            return "—" if val is None else f"{val[0]} × {val[1]}"
        lines.append(f"| `{w['name']}` | {cell(w['option_id'])} | "
                     f"`{w['command_bytes_hex']}` | {pair(w['page_dimensions'])} | "
                     f"{pair(w['printable_area'])} | {pair(w['printable_origin'])} |")
    lines.extend(["", "## Grenzen", "",
                  "- Exakte ID-Gleichheit allein beweist keine Gleichheit der Papierkassette.",
                  "- Windows CUSTOMSIZE hat keine feste Papierhöhe; keine pauschale Längenobergrenze.",
                  "- GPD-rcNameID ist nur eine Ressourcenkennung, kein Anzeigename.",
                  "- Raster-/Seitengröße, bedruckbare Fläche und physische Materialbreite unterscheiden sich.",
                  "- Automatischer Schnitt und Unidrv-/COM-Aufrufreihenfolge werden hier nicht rekonstruiert.", ""])
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_windows_zip", type=Path)
    ap.add_argument("pinned_development_media_json", type=Path)
    args = ap.parse_args(argv)
    options = load_windows_original(args.original_windows_zip)
    print(render(options, read_ppd(LINUX), read_ppd(MACOS),
                 read_development(args.pinned_development_media_json)), end="")


if __name__ == "__main__":
    main()
