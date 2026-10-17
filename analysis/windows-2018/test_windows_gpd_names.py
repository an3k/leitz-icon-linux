#!/usr/bin/env python3
"""Pin the names-only Windows GPD findings without guessing OEM options.

The original 77-option GPD was checked against its archived full extraction
during reconstruction, but OEM numerical OptionIDs and byte commands must
not be inferred from the development catalog.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LINUX = ROOT / "originals/linux-1.0.3/LeitzIcon.ppd"
MAC = ROOT / "originals/macos-5.1.465-classic/LeitzIcon.ppd"
REPORT = ROOT / "analysis/windows-2018/WINDOWS_2018_ALL_MEDIA_NAMES.md"

# Result of prior 77/77 direct GPD-to-inventory field comparison:
DEV_ONLY_IN_WINDOWS_GPD = frozenset((
    "Continuous112", "LargePendaflexTabRotated", "LeverArchFile1005",
    "Multipurpose70030001Rotated", "NameBadgeCard70190001Rotated",
    "ShippingLabel70180001Rotated", "LeitzContinuous112",
    "LeitzDHLShippingLabel", "LeitzNameBadgeCard8214", "LeitzWristband"
))
PAGE = re.compile(r"^\*PageSize[ \t]+([A-Za-z0-9_]+)\/([^:\r\n]*):")
ROW = re.compile(r"^\| \x60([A-Za-z0-9_]+)\x60 \| ")
BT = chr(96)

def original_names(path):
    items = {}
    for line in path.read_text(encoding="latin-1").splitlines():
        match = PAGE.match(line)
        if match:
            ident, label = match.groups()
            if ident in items:
                raise ValueError("duplicate original format: " + ident)
            items[ident] = label
    return items

def value(s):
    return "—" if s is None else str(s).replace("|", r"\|")

def validate(driver_json):
    lin, mac = original_names(LINUX), original_names(MAC)
    records = json.loads(Path(driver_json).read_text(encoding="utf-8"))
    dev = {}
    for item in records:
        name = item["name"]
        if name in dev:
            raise ValueError("duplicate developer format " + name)
        dev[name] = item
    assert (len(lin), len(mac), len(dev)) == (40, 41, 86)
    assert len(DEV_ONLY_IN_WINDOWS_GPD) == 10
    assert DEV_ONLY_IN_WINDOWS_GPD <= set(dev)
    win_fixed = set(dev) - DEV_ONLY_IN_WINDOWS_GPD
    win = win_fixed | {"CUSTOMSIZE"}
    assert (len(win_fixed), len(win)) == (76, 77)
    assert "DHLShippingLabel" in win
    assert set(lin) <= set(mac) <= set(dev)
    names = set(dev) | {"CUSTOMSIZE"}
    lines = {}
    for line in REPORT.read_text(encoding="utf-8").splitlines():
        match = ROW.match(line)
        if match:
            name = match.group(1)
            if name in lines:
                raise ValueError("duplicate result row " + name)
            lines[name] = line
    assert set(lines) == names, (sorted(names - lines.keys()), sorted(lines.keys() - names))
    for name in names:
        d = dev.get(name)
        w = name in win
        geometry = (" / ".join(" × ".join(map(str, d[k]))
                     for k in ("page", "area", "origin"))
                    if w and d else "—")
        win_id = BT + name + BT if w else "—"
        oid = "335" if name == "DHLShippingLabel" else "—" if name == "CUSTOMSIZE" else "nicht gesichert"
        expected = ("| " + " | ".join((
            BT + name + BT, value(lin.get(name)), value(mac.get(name)),
            win_id, oid, value(d["label"] if d else None), geometry
        )) + " |")
        assert lines[name] == expected, name
    print("PASS: 77 Windows original names, 76 matched development IDs, 10 absent Windows GPD IDs, 87 rows")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: test_windows_gpd_names.py PINNED_DRIVER_MEDIA_JSON")
    validate(sys.argv[1])
