#!/usr/bin/env python3
"""Read-only validation of exact media names from distinct source branches."""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LINUX = ROOT / "originals/linux-1.0.3/LeitzIcon.ppd"
MACOS = ROOT / "originals/macos-5.1.465-classic/LeitzIcon.ppd"
REPORT = ROOT / "analysis/windows-2018/PAPER_FORMATS_CROSS_SOURCE.md"
PAGE = re.compile(r"^\*PageSize[ \t]+([A-Za-z0-9_]+)\/([^:\r\n]*):")
ROW = re.compile(r"^\| \x60([A-Za-z0-9_]+)\x60 \| (.*) \|$")
BT = chr(96)

def read_ppd(path):
    found = {}
    for line in path.read_text(encoding="latin-1").splitlines():
        m = PAGE.match(line)
        if m:
            key, name = m.groups()
            if key in found:
                raise ValueError("duplicate PPD ID: " + key)
            found[key] = name
    return found

def fmt(value):
    return "—" if value is None else str(value).replace("|", r"\|").replace("\n", " ")

def check(driver_path):
    linux, macos = read_ppd(LINUX), read_ppd(MACOS)
    records = json.loads(Path(driver_path).read_text(encoding="utf-8"))
    dev = {}
    for record in records:
        key = record["name"]
        if key in dev:
            raise ValueError("duplicate development ID: " + key)
        dev[key] = record["label"]
    assert (len(linux), len(macos), len(dev)) == (40, 41, 86)
    assert set(linux) <= set(macos) <= set(dev)
    assert set(macos) - set(linux) == {"DHLShippingLabel"}
    assert len(set(dev) - set(macos)) == 45
    assert linux["Continuous70050001"] == 'Continuous - 2 1/5"'
    assert macos["Continuous70050001"] == 'Continuous - 2 1/4"'
    dhl = next(x for x in records if x["name"] == "DHLShippingLabel")
    assert "not present in Windows GPD" in dhl["source"]
    assert dhl["page"] == [1039,4204]
    assert dhl["area"] == [960,4144]
    assert dhl["origin"] == [40,30]
    lines = {}
    for line in REPORT.read_text(encoding="utf-8").splitlines():
        m = ROW.match(line)
        if m:
            key = m.group(1)
            if key in lines:
                raise ValueError("duplicate report ID: " + key)
            lines[key] = line
    names = set(dev) | {"CUSTOMSIZE"}
    assert set(lines) == names, (sorted(names - set(lines)), sorted(set(lines) - names))
    for key in sorted(names):
        win = BT + key + BT if key in ("DHLShippingLabel", "CUSTOMSIZE") else "—"
        expected = (f"| {BT}{key}{BT} | {fmt(linux.get(key))} | "
                    f"{fmt(macos.get(key))} | {win} | {fmt(dev.get(key))} |")
        assert lines[key] == expected, ("mismatched inventory row: " + key)
    print("PASS: Linux 40; macOS 41; development 86; named Windows 2 of 77.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: test_paper_cross_sources.py DRIVER_MEDIA_JSON")
    check(sys.argv[1])
