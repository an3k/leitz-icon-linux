#!/usr/bin/env python3
"""Read actual manufacturer Windows ZIP and prove LeitzRp WriteData RVAs.

No third-party packages or execution of OEM DLLs. Optional --linux / --macos
verify the unmodified source artifacts used for the four-source comparison.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA = {
    "windows_zip": "cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca",
    "x86": "7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33",
    "x64": "876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787",
    "linux": "bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0",
    "macos": "72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13",
}
# Original PE .text opcode sequences, not reverse-engineered C or signatures
# built from the new implementation. Functions are RVAs, not file offsets.
PROOFS = {
    "x86": {
        0x2a11: "01 78 0c",           # original 32-bit accumulated-byte ADD
        0x2a9c: "ff 51 10",           # COM host writing method at vtable +0x10
        0x2aa7: "3d 05 40 00 80",     # E_FAIL comparison, not general FAILED(hr)
        0x2ac6: "68 c8 00 00 00",     # failure reporting code 200
        0x2ad6: "ff 51 28",           # COM host error callback +0x28
        0x1554: "0f b7 40 0c",        # original medium WORD +0x0c
        0x1558: "8d 04 80",           # 5x
        0x155b: "83 c0 10",           # +16
        0x1585: "6a 01",              # send exactly one byte
        0x158f: "8b ce",              # x86 thiscall setup
        0x1591: "e8 5a 14 00 00",     # CALL original 0x29f0
    },
    "x64": {
        0x2a00: "4c 01 49 18",        # original 64-bit accumulated-byte ADD
        0x2aa1: "ff 50 20",           # COM host writer +0x20
        0x2ab5: "3d 05 40 00 80",     # exact E_FAIL-only check
        0x2ad0: "c7 44 24 20 c8 00 00 00", # error code 200
        0x2ad8: "41 ff 52 50",        # COM host error callback +0x50
        0x156d: "0f b7 50 0c",        # original medium WORD +0x0c
        0x1571: "48 8d 42 04",        # word+4
        0x1575: "48 8d 04 82",        # 5*word+16
        0x15a0: "41 b9 01 00 00 00",  # send exactly one byte
        0x15ac: "e8 2f 14 00 00",     # CALL original 0x29e0
    },
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def pe_sections(raw):
    pe = struct.unpack_from("<I", raw, 0x3c)[0]
    if raw[pe:pe+4] != b"PE\0\0":
        raise ValueError("Not an original PE binary")
    sections = struct.unpack_from("<H", raw, pe+6)[0]
    optional_size = struct.unpack_from("<H", raw, pe+20)[0]
    section_table = pe+24+optional_size
    result=[]
    for i in range(sections):
        off=section_table+40*i
        name=raw[off:off+8].split(b"\0",1)[0]
        virt_size, virt_addr, raw_size, raw_addr = struct.unpack_from("<IIII",raw,off+8)
        result.append((name,virt_addr,raw_addr,raw_size))
    return result


def read_rva(raw,sections,rva,length):
    for name,virt,raw_start,raw_size in sections:
        if virt<=rva and rva+length<=virt+raw_size:
            return raw[raw_start+rva-virt:raw_start+rva-virt+length]
    raise ValueError(f"Unmapped OEM RVA 0x{rva:x}")


def extract_unique(z,end):
    paths=[p for p in z.namelist() if p.endswith(end)]
    if len(paths)!=1:
        raise ValueError(f"Expected exactly one OEM file ending {end}: {paths[:5]}")
    return z.read(paths[0])


def check_sha_zip(zip_name,end,expected):
    with zipfile.ZipFile(zip_name) as z:
        data=extract_unique(z,end)
    if sha(data)!=expected:
        raise ValueError(f"SHA-256 mismatch for {end}")
    print(f"PASS: unchanged original {end}")


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--windows",type=Path,required=True)
    ap.add_argument("--linux",type=Path)
    ap.add_argument("--macos",type=Path)
    args=ap.parse_args()
    if sha(args.windows.read_bytes())!=SHA['windows_zip']:
        raise ValueError("Original Windows ZIP checksum differs")
    with zipfile.ZipFile(args.windows) as z:
        for arch in ("x86","x64"):
            directory="i386" if arch=="x86" else "amd64"
            raw=extract_unique(z,"/Drivers/"+directory+"/LeitzRp.dll")
            if sha(raw)!=SHA[arch]:
                raise ValueError("Original "+arch+" raster DLL changed")
            sec=pe_sections(raw)
            for rva,code in PROOFS[arch].items():
                expected=bytes.fromhex(code)
                if read_rva(raw,sec,rva,len(expected))!=expected:
                    raise ValueError(f"{arch} byte mismatch at RVA 0x{rva:x}")
            print(f"PASS: original Windows {arch} LeitzRp WriteData + slot8: {len(PROOFS[arch])} native opcode checks")
    if args.linux:
        check_sha_zip(args.linux,"/raster2printer.c",SHA['linux'])
    if args.macos:
        check_sha_zip(args.macos,"/LeitzPrint.bundle/Contents/MacOS/raster2printer",SHA['macos'])

if __name__=="__main__":
    main()
