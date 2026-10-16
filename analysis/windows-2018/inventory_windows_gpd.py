#!/usr/bin/env python3
"""Inventorisiert die originale Windows-2018-GPD direkt aus dem Nutzer-ZIP.
Niemals Installer/DLL ausführen. Ausgabe: vollständiges JSON auf stdout.
Aufruf: python3 inventory_windows_gpd.py LeitzIconLabelSetup.zip > gpd.json
"""
import hashlib
import json
import re
import sys
import zipfile

OPTION = re.compile(r'(?m)^\s*\*Option:\s*([A-Za-z0-9_]+)\s*\r?\n\s*\{')

def balanced_block(source, brace):
    depth = 0
    quote = False
    for i in range(brace, len(source)):
        char = source[i]
        if char == '"' and (i == 0 or source[i-1] != '\\'):
            quote = not quote
        if quote:
            continue
        if char == '{':
            depth += 1
        elif char == '}':
            depth -= 1
            if depth == 0:
                return source[brace+1:i]
    raise ValueError("Unterbrochener GPD-Optionsblock")

def command_bytes(raw):
    out = bytearray()
    for part in re.finditer(r'<([0-9a-fA-F]{2,})>|([^<]+)',raw):
        if part.group(1):
            out.extend(bytes.fromhex(part.group(1)))
        else:
            out.extend(part.group(2).encode('ascii'))
    return bytes(out)

def paper_options(gpd):
    start = gpd.index('*Feature: PaperSize')
    stop = gpd.index('*Feature: Halftone',start)
    source = gpd[start:stop]
    output = []
    for m in OPTION.finditer(source):
        body = balanced_block(source,m.end()-1)
        def integer(key):
            found = re.search(r'\*'+key+r':\s*(\d+)',body)
            return int(found.group(1)) if found else None
        def pair(key):
            found = re.search(r'\*'+key+r':\s*PAIR\((\d+),\s*(\d+)\)',body)
            return [int(found.group(1)),int(found.group(2))] if found else None
        cmd = re.search(r'\*Cmd:\s*"([^"]*)"',body)
        if not cmd:
            raise ValueError("Papier ohne CmdSelect: "+m.group(1))
        output.append({
            "name":m.group(1),
            "option_id":integer("OptionID"),
            "resource_name_id":integer("rcNameID"),
            "page_dimensions":pair("PageDimensions"),
            "printable_area":pair("PrintableArea"),
            "printable_origin":pair("PrintableOrigin"),
            "min_size":pair("MinSize"),
            "max_size":pair("MaxSize"),
            "raw_command":cmd.group(1),
            "command_bytes_hex":command_bytes(cmd.group(1)).hex(),
        })
    return output

def main(archive):
    prefix='LeitzIconLabelSetup/SourceDir/Leitz Icon Software/Drivers/'
    with zipfile.ZipFile(archive) as f:
        gpd=f.read(prefix+'LeitzIcon.gpd')
        inf=f.read(prefix+'OEMPRINT.INF')
    entries=paper_options(gpd.decode('ascii'))
    assert len(entries)==77 and len({x["name"] for x in entries})==77
    assert sum(x["page_dimensions"] is not None for x in entries)==76
    assert sum(x["name"]=="CUSTOMSIZE" for x in entries)==1
    assert len({x["option_id"] for x in entries if x["option_id"] is not None})==76
    version=re.search(rb'DriverVer=([^\r\n]+)',inf)
    print(json.dumps({
        "source":prefix+"LeitzIcon.gpd",
        "gpd_sha256":hashlib.sha256(gpd).hexdigest(),
        "inf_sha256":hashlib.sha256(inf).hexdigest(),
        "driver_version_line":version.group(1).decode() if version else None,
        "original_win_fixed_count":76,
        "original_win_custom_count":1,
        "original_win_total_count":77,
        "units":"Original-GPD MasterUnits PAIR(300,600), nicht mm",
        "options":entries
    },ensure_ascii=False,indent=2))

if __name__=="__main__":
    if len(sys.argv)!=2:
        sys.exit("Aufruf: python3 inventory_windows_gpd.py ORIGINAL.zip")
    main(sys.argv[1])
