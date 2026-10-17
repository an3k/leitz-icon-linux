#!/usr/bin/env python3
"""Regenerate verified OEM Windows GPD media entries, without guessing IDs.

Reads the preserved 77-option inventory AND independently reparses the
preserved GPD text transcription. Does not touch original manufacturer files.
"""
import argparse
import json
from pathlib import Path
from inventory_windows_gpd import paper_options

HERE = Path(__file__).resolve().parent
OUT = HERE.parents[1] / "reconstructions/windows-2018/GPD/gpd_media_catalog_entries.inc"
INVENTORY = HERE / "windows_2018_original_media_inventory.json"
GPD_TEXT = HERE / "LeitzIcon_2018_GPD_text_transcription.gpd.txt"

def pair(v):
    return "{-1, -1}" if v is None else "{" + str(v[0]) + ", " + str(v[1]) + "}"

def command(h):
    values = [f"0x{h[i:i+2].lower()}" for i in range(0, len(h), 2)]
    if len(values) > 8:
        raise ValueError("Original GPD media command longer than 8 bytes")
    return "{" + ", ".join(values + ["0x00"] * (8 - len(values))) + "}"

def generate():
    saved = json.loads(INVENTORY.read_text(encoding="utf-8"))
    parsed = paper_options(GPD_TEXT.read_text(encoding="utf-8"))
    if parsed != saved["options"]:
        raise ValueError("Archived Windows GPD transcription and original extraction disagree")
    if len(parsed) != 77 or sum(x["page_dimensions"] is not None for x in parsed) != 76:
        raise ValueError("Expected 76 fixed GPD options and CUSTOMSIZE")
    if len({x["name"] for x in parsed}) != 77:
        raise ValueError("Duplicate OEM media identifier")
    rows = ["/* GENERATED from original GPD-verified windows_2018_original_media_inventory.json. */"]
    for o in parsed:
        quoted = json.dumps(o["name"],ensure_ascii=True)
        fields = [quoted,str(o["option_id"] if o["option_id"] is not None else -1),
                  str(o["resource_name_id"] if o["resource_name_id"] is not None else -1)]
        fields += [pair(o[k]) for k in (
            "page_dimensions","printable_area","printable_origin","min_size","max_size")]
        fields += [command(o["command_bytes_hex"]),str(len(o["command_bytes_hex"])//2)]
        rows.append("    {" + ", ".join(fields) + "},")
    return "\n".join(rows) + "\n"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check",action="store_true")
    args = parser.parse_args()
    body = generate()
    if args.check:
        if not OUT.exists() or OUT.read_text(encoding="utf-8") != body:
            raise SystemExit("ERROR: GPD C catalog is not synchronized with preserved OEM GPD")
        print("PASS: 77 OEM GPD media entries, C catalog byte-for-byte in sync")
    else:
        OUT.write_text(body, encoding="utf-8")
        print("Generated", OUT)

if __name__ == "__main__":
    main()
