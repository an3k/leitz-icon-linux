#!/usr/bin/env python3
"""Check recovered OEM Win32 RT_STRING names against 77 GPD option IDs."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
source=json.loads((HERE/"windows_2018_original_ui_strings.json").read_text(encoding="utf-8"))
inventory=json.loads((HERE/"windows_2018_original_media_inventory.json").read_text(encoding="utf-8"))
report=(HERE/"FOUR_SOURCE_WITH_WINDOWS_UI_NAMES.md").read_text(encoding="utf-8")
assert source["language_id"]==1033 and source["resource_count"]==77
assert source["fixed_paper_count"]==76 and len(source["fixed_paper_strings"])==76
assert source["sha256"]["LeitzIcon.gpd"]==inventory["gpd_sha256"]
byname={x["gpd_name"]:x for x in source["fixed_paper_strings"]}
assert len(byname)==76
fixed=[o for o in inventory["options"] if o["page_dimensions"] is not None]
assert len(fixed)==76
for row in fixed:
    label=byname[row["name"]]
    assert label["rcNameID"]==row["resource_name_id"]==row["option_id"]
    assert label["windows_display_name"]
    matches=[line for line in report.splitlines()
             if line.startswith("| `"+row["name"]+"` |")]
    assert len(matches)==1
    cols=matches[0].split(" | ")
    assert len(cols)==9 and cols[4]==label["windows_display_name"]
assert byname["DHLShippingLabel"]["windows_display_name"]=='*Package Label 7"x3.5"'
assert byname["NameBadgeCard8214"]["windows_display_name"]=="Name Badge Card - Durable© 8214"
assert source["unreferenced_resource_strings"]==[{"id":333,"text":"*Custom paper size"}]
assert source["customsize"]["rcNameID_symbol"]=="=USER_DEFINED_SIZE_DISPLAY"
assert source["customsize"]["windows_display_name"] is None
assert not any(x["rcNameID"]==333 for x in source["fixed_paper_strings"])
assert "| `CUSTOMSIZE` |" in report
print("PASS: 76 Windows OEM paper UI strings, 77 PE resources, GPD mappings and cross-source report")
