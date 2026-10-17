#!/usr/bin/env python3
"""Rebuild exact 77-entry C PE UI string table from preserved OEM resources."""
import argparse
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
UI=HERE/"windows_2018_original_ui_strings.json"
OEM=HERE/"windows_2018_original_media_inventory.json"
OUT=ROOT/"reconstructions/windows-2018/GPD/gpd_ui_resources_entries.inc"
def generate():
    ui=json.loads(UI.read_text(encoding="utf-8"))
    inventory=json.loads(OEM.read_text(encoding="utf-8"))
    names={}
    fixed=ui["fixed_paper_strings"]
    assert len(fixed)==76 and len(inventory["options"])==77
    byname={item["gpd_name"]:item for item in fixed}
    for entry in inventory["options"]:
        if entry["name"]=="CUSTOMSIZE":
            assert entry["resource_name_id"] is None
        else:
            row=byname[entry["name"]]
            assert row["rcNameID"]==entry["resource_name_id"]
    for item in fixed:
        idx=item["rcNameID"]
        if idx in names:raise ValueError("duplicate PE resource")
        names[idx]=item["windows_display_name"]
    for item in ui["unreferenced_resource_strings"]:
        idx=item["id"]
        if idx in names:raise ValueError("duplicate unreferenced resource")
        names[idx]=item["text"]
    if set(names)!=set(range(259,336)) or len(fixed)!=76:
        raise ValueError("wrong OEM Windows resource ID set")
    result=["/* Original PE RT_STRING language 0x0409; regenerated from pinned JSON. */"]
    for idx in range(259,336):
        result.append("    "+json.dumps(names[idx],ensure_ascii=False)+f", /* {idx} */")
    return "\n".join(result)+"\n"
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--check",action="store_true")
    opt=ap.parse_args()
    result=generate()
    if opt.check:
        if OUT.read_text(encoding="utf-8")!=result:
            raise SystemExit("C UI labels table out of sync with original PE")
        print("PASS: 77 OEM RT_STRING IDs and 76 GPD names aligned")
    else:
        OUT.write_text(result,encoding="utf-8")
if __name__=="__main__":
    main()
