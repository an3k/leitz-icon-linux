#!/usr/bin/env python3
"""Read-only consistency check for the legacy v0.4.1 media catalog.

This branch has 76 media entries, unlike the development branch's 86.
Keep original identifiers and the existing PPD display names unchanged.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MEDIA_PATH = ROOT / "media/media.json"
PPD_PATH = ROOT / "cups/LeitzIcon.ppd"
HEADER_PATH = ROOT / "src/leitz_media.h"

def fail(message: str) -> None:
    raise SystemExit(f"ERROR: {message}")

def check() -> None:
    media = json.loads(MEDIA_PATH.read_text(encoding="utf-8"))
    if len(media) != 76:
        fail(f"Expected 76 v0.4.1 media profiles, found {len(media)}")
    names = [item["name"] for item in media]
    if len(set(names)) != 76:
        fail("Duplicate media identifiers")

    header = HEADER_PATH.read_text(encoding="utf-8")
    entries = re.findall(r'^\s*\{"([^"]+)",([0-9,]+)\},\s*$', header, re.MULTILINE)
    if len(entries) != 76:
        fail(f"C header contains {len(entries)} profiles, expected 76")
    if [name for name, _ in entries] != names:
        fail("Header media names or order differ from the JSON catalog")
    for item, (_, numbers) in zip(media, entries):
        actual = [int(n) for n in numbers.split(",")]
        expected = [
            *item["page"], *item["area"], *item["origin"],
            item["command_length"], int(item["continuous"])
        ]
        if actual != expected:
            fail(f"C header geometry differs for {item['name']}")

    ppd = PPD_PATH.read_text(encoding="latin-1")
    for key in ("PageSize", "PageRegion", "ImageableArea", "PaperDimension"):
        observed = re.findall(
            rf'^\*{key} ([A-Za-z0-9_]+)(?:/[^:]+)?:',
            ppd, re.MULTILINE
        )
        if observed != names:
            fail(f"PPD {key} names/order differ from the media catalog")
    if '*FileVersion: "0.4.1"' not in ppd:
        fail("Unexpected PPD version; this check targets v0.4.1")
    if "*CustomPageSize True:" not in ppd:
        fail("Custom-page support missing from PPD")

    pages = re.findall(
        r'^\*PageSize ([A-Za-z0-9_]+)/[^:]+: '
        r'"<</PageSize\[([0-9.]+) ([0-9.]+)\]>>setpagedevice"',
        ppd, re.MULTILINE
    )
    if len(pages) != 76:
        fail(f"Expected 76 explicit PPD PageSize geometries, found {len(pages)}")
    for item, (name, width, height) in zip(media, pages):
        expected_width = item["page"][0] * 72 / 300
        expected_height = item["page"][1] * 72 / 600
        if (name != item["name"] or
                abs(float(width) - expected_width) > 0.001 or
                abs(float(height) - expected_height) > 0.001):
            fail(f"PPD page geometry differs for {item['name']}")
    print("PASS: 76 legacy media profiles match JSON, C header and PPD")

if __name__ == "__main__":
    check()
