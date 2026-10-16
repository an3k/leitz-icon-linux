#!/usr/bin/env python3
"""Inventory the OEM Windows application LabelSizes.xml separately from its GPD.

Usage: python3 inventory_windows_label_sizes.py LeitzIconLabelSetup.zip
No installer, DLL, device or firmware operation.
"""
from fractions import Fraction
import hashlib
import json
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

from inventory_windows_gpd import paper_options

PREFIX = "LeitzIconLabelSetup/SourceDir/Leitz Icon Software/"
PX_X, PX_Y = 300, 600
MM_PER_IN = Fraction(254, 10)
TOLERANCE_MM = Fraction(1, 1)

def optional_integer(text):
    clean = (text or "").strip()
    if not clean:
        return None
    value = int(clean)
    if value <= 0:
        raise ValueError("Nonpositive OEM dimension")
    return value

def parse_inch(text):
    clean = (text or "").strip()
    if not clean:
        return None
    return sum((Fraction(s) for s in clean.split()), Fraction())

def collect_ui(xml_bytes):
    root = ET.fromstring(xml_bytes)
    records = []
    names = set()
    for item in root.findall("./LabelSizes/LabelSize"):
        name = item.get("Id")
        if not name or name in names:
            raise ValueError("Missing or duplicate LabelSizes Id: "+str(name))
        names.add(name)
        width = optional_integer(item.findtext("Metre/Width"))
        height = optional_integer(item.findtext("Metre/Height"))
        inch_width = (item.findtext("Inch/Width") or "").strip()
        inch_height = (item.findtext("Inch/Height") or "").strip()
        iw = parse_inch(inch_width)
        ih = parse_inch(inch_height)
        if width is None or iw is None or ((height is None) != (ih is None)):
            raise ValueError("Incomplete original UI dimensions: "+name)
        if height is None:
            comparison = "continuous_no_fixed_height"
        else:
            direct = max(abs(Fraction(width)-iw*MM_PER_IN),
                         abs(Fraction(height)-ih*MM_PER_IN))
            swapped = max(abs(Fraction(width)-ih*MM_PER_IN),
                          abs(Fraction(height)-iw*MM_PER_IN))
            comparison = ("direct" if direct <= TOLERANCE_MM else
                          "swapped" if swapped <= TOLERANCE_MM else "different")
        records.append({
            "id":name, "application_width_mm":width,
            "application_height_mm":height,
            "original_inch_width":inch_width,
            "original_inch_height":inch_height,
            "metric_to_inch_orientation":comparison
        })
    return records

def classify(ui, gpd):
    if gpd is None:
        return {"gpd_presence":"missing","orientation":"not_in_gpd",
                "gpd_option_id":None,"gpd_page_dimensions":None,
                "gpd_printable_area":None,"gpd_printable_origin":None}
    dims = gpd["page_dimensions"]
    if dims is None:
        return {"gpd_presence":"custom","orientation":"undefined",
                "gpd_option_id":gpd["option_id"],"gpd_page_dimensions":None,
                "gpd_printable_area":None,"gpd_printable_origin":None}
    if ui["application_height_mm"] is None:
        orientation = "continuous_not_comparable"
    else:
        x=Fraction(dims[0],PX_X)*MM_PER_IN
        y=Fraction(dims[1],PX_Y)*MM_PER_IN
        w=Fraction(ui["application_width_mm"])
        h=Fraction(ui["application_height_mm"])
        direct=max(abs(x-w),abs(y-h))
        swapped=max(abs(x-h),abs(y-w))
        orientation=("direct" if direct<=TOLERANCE_MM else
                     "swapped" if swapped<=TOLERANCE_MM else "different")
    return {"gpd_presence":"present","orientation":orientation,
            "gpd_option_id":gpd["option_id"],
            "gpd_page_dimensions":dims,
            "gpd_printable_area":gpd["printable_area"],
            "gpd_printable_origin":gpd["printable_origin"]}

def inventory(label_xml, gpd_bytes):
    gpd_text=gpd_bytes.decode("ascii")
    if not re.search(r"\*MasterUnits:\s*PAIR\(\s*300\s*,\s*600\s*\)",gpd_text):
        raise ValueError("Unexpected GPD units; conversion not supported")
    options=paper_options(gpd_text)
    option_by_name={o["name"]:o for o in options}
    if len(options)!=len(option_by_name):
        raise ValueError("Duplicate GPD option identifiers")
    ui=collect_ui(label_xml)
    for item in ui:
        item.update(classify(item,option_by_name.get(item["id"])))
    names={u["id"] for u in ui}
    gpd_only=[g["name"] for g in options if g["name"] not in names]
    counts={
        "application_xml_count":len(ui),
        "gpd_option_count":len(options),
        "gpd_fixed_count":sum(x["page_dimensions"] is not None for x in options),
        "gpd_custom_count":sum(x["page_dimensions"] is None for x in options),
        "shared_exact_id_count":sum(x["gpd_presence"]!="missing" for x in ui),
        "ui_only_count":sum(x["gpd_presence"]=="missing" for x in ui),
        "gpd_only_count":len(gpd_only),
        "ui_continuous_count":sum(x["application_height_mm"] is None for x in ui),
        "shared_fixed_direct":sum(x["orientation"]=="direct" for x in ui),
        "shared_fixed_swapped":sum(x["orientation"]=="swapped" for x in ui),
        "shared_fixed_different":sum(x["orientation"]=="different" for x in ui),
        "inch_metric_rotated":sum(x["metric_to_inch_orientation"]=="swapped" for x in ui)
    }
    return {
        "application_xml_source":PREFIX+"LabelSizes.xml",
        "application_xml_sha256":hashlib.sha256(label_xml).hexdigest(),
        "driver_gpd_source":PREFIX+"Drivers/LeitzIcon.gpd",
        "driver_gpd_sha256":hashlib.sha256(gpd_bytes).hexdigest(),
        "gpd_master_units":[PX_X,PX_Y],
        "matching_tolerance_mm":float(TOLERANCE_MM),
        "notes":[
            "UI label dimensions and GPD page dimensions may have different orientations.",
            "GPD page/printable values are printer units, not millimetres.",
            "Missing from Windows GPD does not mean unsupported physical stock.",
            "A GPD continuous-media page height is not a hardware length limit.",
            "Matching identifiers do not themselves prove identical physical cassettes."
        ],
        "counts":counts,
        "application_labels":ui,
        "gpd_only_identifiers":gpd_only
    }

def main(filename):
    with zipfile.ZipFile(filename) as z:
        data=inventory(z.read(PREFIX+"LabelSizes.xml"),
                       z.read(PREFIX+"Drivers/LeitzIcon.gpd"))
    print(json.dumps(data,ensure_ascii=False,indent=2))

if __name__=="__main__":
    if len(sys.argv)!=2:
        sys.exit("Usage: python3 inventory_windows_label_sizes.py ORIGINAL.zip")
    main(sys.argv[1])
