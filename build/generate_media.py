#!/usr/bin/env python3
"""Generate the authoritative 86 media formats into C and CUPS PPD.
The protocol choices are static; installed protocol modules do not alter PPD.
"""
import argparse
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
MEDIA=ROOT/"media/media.json"
TEMPLATE=ROOT/"cups/LeitzIcon.ppd.in"
PPD=ROOT/"cups/LeitzIcon.ppd"
HEADER=ROOT/"src/leitz_media.h"
MARKERS={
 "PageSize":"@LEITZ_PAGE_SIZE@",
 "PageRegion":"@LEITZ_PAGE_REGION@",
 "ImageableArea":"@LEITZ_IMAGEABLE_AREA@",
 "PaperDimension":"@LEITZ_PAPER_DIMENSION@",
}

def fmt(value):
    return f"{value:.4f}"

def generate():
    catalog=json.loads(MEDIA.read_text(encoding="utf-8"))
    if len(catalog)!=86 or len({m["name"] for m in catalog})!=86:
        raise ValueError("MASTER catalog must contain 86 unique media IDs")
    records={kind:[] for kind in MARKERS}
    hdr=[
      "/* GENERATED from media/media.json; do not edit. */",
      "#ifndef LEITZ_MEDIA_H",
      "#define LEITZ_MEDIA_H",
      "struct leitz_media {const char *name; unsigned page_width,page_height,raster_width,raster_height,left,bottom,length_code; int continuous;};",
      "static const struct leitz_media leitz_media_catalog[] = {",
    ]
    for m in catalog:
        name=m["name"]
        label=m["label"]
        if not name.isascii() or not name.replace("_","").isalnum() or not label:
            raise ValueError(f"Invalid CUPS media name or label: {name}")
        pw,ph=(fmt(m["page"][0]*72/300),fmt(m["page"][1]*72/600))
        box=f"<</PageSize[{pw} {ph}]>>setpagedevice"
        records["PageSize"].append(f'*PageSize {name}/{label}: "{box}"')
        records["PageRegion"].append(f'*PageRegion {name}: "{box}"')
        x0,y0=(fmt(m["origin"][0]*72/300),fmt(m["origin"][1]*72/600))
        x1,y1=(fmt((m["origin"][0]+m["area"][0])*72/300),
               fmt((m["origin"][1]+m["area"][1])*72/600))
        records["ImageableArea"].append(f'*ImageableArea {name}: "{x0} {y0} {x1} {y1}"')
        records["PaperDimension"].append(f'*PaperDimension {name}: "{pw} {ph}"')
        hdr.append('  {'+json.dumps(name)+','+','.join(str(v) for v in (
            *m["page"],*m["area"],*m["origin"],m["command_length"],
            int(m["continuous"])))+'},')
    hdr.extend(["};",
                "#define LEITZ_MEDIA_COUNT (sizeof(leitz_media_catalog)/sizeof(leitz_media_catalog[0]))",
                "#endif",""])
    ppd=TEMPLATE.read_text(encoding="utf-8")
    for kind,token in MARKERS.items():
        if ppd.count(token)!=1:
            raise ValueError(f"Missing or duplicate PPD marker: {token}")
        ppd=ppd.replace(token,"\n".join(records[kind]))
    if "*DefaultLeitzProtocol: Auto" not in ppd:
        raise ValueError("Auto protocol must be the static PPD default")
    return {HEADER:"\n".join(hdr),PPD:ppd}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--check",action="store_true")
    args=ap.parse_args()
    for path,content in generate().items():
        if args.check:
            if not path.exists() or path.read_text(encoding="utf-8")!=content:
                raise SystemExit(f"ERROR: {path} is not synchronized with media.json")
        else:
            path.write_text(content,encoding="utf-8")
        print(("CHECK " if args.check else "GENERATED ")+str(path.relative_to(ROOT)))
if __name__=="__main__":
    main()
