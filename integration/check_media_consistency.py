#!/usr/bin/env python3
"""Offline integrity gate for the A/B Leitz Icon media catalog.

Read-only. Never accesses USB, CUPS, or printer hardware.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
catalog = json.loads((ROOT / "media/media.json").read_text(encoding="utf-8"))
ppd = (ROOT / "driver/cups" / "LeitzIcon.ppd").read_text(encoding="latin-1")
header = (ROOT / "driver/src" / "leitz_media.h").read_text(encoding="utf-8")


assert len(catalog) == 86, "Exactly 86 media profiles expected"
assert len({m["name"] for m in catalog}) == 86, "Duplicate media IDs"
left, right = catalog[:43], catalog[43:]
assert all(not m["name"].startswith("Leitz") for m in left)
assert [m["name"] for m in right] == ["Leitz" + m["name"] for m in left]
assert [m["name"].lower() for m in left] == sorted(m["name"].lower() for m in left)

def section(option):
    if option == "PageSize":
        regex = r'^\*PageSize (\w+)/([^:]+): "<</PageSize\[([0-9.]+) ([0-9.]+)\]>>setpagedevice"$'
    elif option == "PageRegion":
        regex = r'^\*PageRegion (\w+): '
    else:
        regex = rf'^\*{option} (\w+): '
    return list(re.finditer(regex, ppd, re.MULTILINE))

media_ids = [m["name"] for m in catalog]
for category in ("PageSize", "PageRegion", "ImageableArea", "PaperDimension"):
    records = section(category)
    assert [m.group(1) for m in records] == media_ids, category

pages = section("PageSize")
assert len(pages) == 86
for item, match in zip(catalog, pages):
    expected_w = item["page"][0] * 72 / 300
    expected_h = item["page"][1] * 72 / 600
    assert abs(float(match.group(3)) - expected_w) < .001, item["name"]
    assert abs(float(match.group(4)) - expected_h) < .001, item["name"]
assert "*CustomPageSize True:" in ppd

for record, match in zip(catalog, pages):
    assert match.group(2) == record["label"], record["name"]
assert sum(m["name"].startswith("Leitz") for m in catalog) == 43

structure = []
for line in header.splitlines():
    line = line.strip()
    if not (line.startswith('{"') and line.endswith('},')):
        continue
    fields = line[1:-2].split(",")
    assert len(fields) == 9, ("Malformed C header media entry", line)
    structure.append((fields[0].strip('"'), *fields[1:]))

assert len(structure) == 86, "C header count mismatch"
for item, record in zip(catalog, structure):
    name, pw, ph, aw, ah, ox, oy, length, continuous = record
    assert item["name"] == name
    assert item["page"] == [int(pw), int(ph)]
    assert item["area"] == [int(aw), int(ah)]
    assert item["origin"] == [int(ox), int(oy)]
    assert item["command_length"] == int(length)
    assert item["continuous"] == bool(int(continuous))

differences = []
for a, b in zip(left, right):
    if any(a[field] != b[field] for field in (
            "page", "area", "origin", "command_length",
            "command_width", "continuous")):
        differences.append(a["name"])
expected = {
    "LargePendaflexTabRotated",
    "Multipurpose70030001Rotated",
    "NameBadgeCard70190001Rotated",
    "ShippingLabel70180001Rotated",
}
assert set(differences) == expected, ("Unexpected source-geometry changes", differences)
print("PASS 43 A + 43 B profiles, 4 native source differences preserved")
print("PASS 86 complete PPD profiles, generated C header, master labels")
