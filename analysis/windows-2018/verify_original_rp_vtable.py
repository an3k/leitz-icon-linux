#!/usr/bin/env python3
"""Verify native 2018 LeitzRp COM vtable (26 slots each for x86/x64).

No proprietary binaries are committed. With --windows-zip, parse the
ORIGINAL PE sections directly; with --linux-zip / --macos-zip confirm
cross-platform original-file SHA256 provenance. Otherwise only check
committed JSON and generated portable C initializer (CI mode).
"""
import argparse
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
RECORDS = HERE / "leitzrp_vtable_original_2018.json"
INC = ROOT / "reconstructions/windows-2018/LeitzRp/oem_method_map_entries.inc"
ROLES = {
    "native_method": "LEITZ_RP_METHOD_NATIVE",
    "fixed_s_ok": "LEITZ_RP_METHOD_S_OK",
    "fixed_one": "LEITZ_RP_METHOD_ONE",
    "filter_name_comparison": "LEITZ_RP_METHOD_FILTER_NAME",
    "e_notimpl": "LEITZ_RP_METHOD_NOTIMPL",
}


def generate(data):
    rows = data["records"]
    if len(rows) != 26 or [r["slot"] for r in rows] != list(range(26)):
        raise ValueError("Expected exact numbered 26-slot OEM table")
    if {i for i, r in enumerate(rows) if r["observed_implementation"] == "e_notimpl"} != {13, 14, *range(16, 26)}:
        raise ValueError("Incorrect original unsupported methods")
    lines = ["/* GENERATED FROM Original LeitzRp.dll x86/x64 vtables; see leitzrp_vtable_original_2018.json */"]
    for row in rows:
        role = ROLES[row["observed_implementation"]]
        lines.append("    {0x%04xu, 0x%04xu, %s}, /* slot %2d */" %
                     (row["x86_rva"], row["x64_rva"], role, row["slot"]))
    return "\n".join(lines) + "\n"


class OriginalPE:
    def __init__(self, raw):
        self.raw = raw
        pe = struct.unpack_from("<I", raw, 0x3c)[0]
        if raw[pe:pe+4] != b"PE\0\0":
            raise ValueError("Missing original Windows PE signature")
        sections, optional_size = struct.unpack_from("<xxH12xH", raw, pe+4)
        # Standard IMAGE_FILE_HEADER: NumberOfSections at +2; SizeOfOptionalHeader at +16.
        self.opt = pe + 24
        self.magic = struct.unpack_from("<H", raw, self.opt)[0]
        if self.magic == 0x10b:
            self.ptrsize = 4
            self.base = struct.unpack_from("<I", raw, self.opt + 28)[0]
        elif self.magic == 0x20b:
            self.ptrsize = 8
            self.base = struct.unpack_from("<Q", raw, self.opt + 24)[0]
        else:
            raise ValueError("Unsupported manufacturer PE optional header")
        self.sections = []
        for i in range(sections):
            off = self.opt + optional_size + 40*i
            name = raw[off:off+8].rstrip(b"\0").decode("ascii")
            virtual_size, vaddr, raw_size, raw_start = struct.unpack_from("<IIII", raw, off+8)
            self.sections.append((name, virtual_size, vaddr, raw_size, raw_start))

    def bytes_at(self, rva, n):
        for name, virtual_size, vaddr, raw_size, start in self.sections:
            if vaddr <= rva and rva+n <= vaddr+raw_size:
                return self.raw[start+(rva-vaddr):start+(rva-vaddr)+n]
        raise ValueError("OEM RVA not mapped to raw section: 0x%x" % rva)

    def vtable(self, rva, count):
        width = self.ptrsize
        block = self.bytes_at(rva, count*width)
        entries = struct.unpack("<"+("Q" if width==8 else "I")*count,block)
        return [x-self.base for x in entries]


def check_windows(zip_path, data):
    blob = Path(zip_path).read_bytes()
    if hashlib.sha256(blob).hexdigest() != data["source"]["original_zip_sha256"]:
        raise ValueError("Original Windows archive SHA256 mismatch")
    with zipfile.ZipFile(zip_path) as z:
        for arch in ("x86", "x64"):
            subdir = "i386" if arch=="x86" else "amd64"
            paths = [s for s in z.namelist() if s.endswith("/Drivers/"+subdir+"/LeitzRp.dll")]
            if len(paths)!=1:
                raise ValueError("Missing or duplicate original Windows LeitzRp "+arch)
            raw = z.read(paths[0])
            assert hashlib.sha256(raw).hexdigest() == data["source"][arch+"_sha256"]
            pe = OriginalPE(raw)
            assert pe.ptrsize == (4 if arch=="x86" else 8)
            got = pe.vtable(data["source"][arch+"_vtable_rva"], 26)
            want = [r[arch+"_rva"] for r in data["records"]]
            if got != want:
                raise ValueError("Original "+arch+" LeitzRp COM vtable changed: "+repr((got,want)))
            assert pe.bytes_at(data["source"][arch+"_filtergraphics_ascii_rva"],15) == b"FilterGraphics\x00"
            for r in data["records"]:
                code = pe.bytes_at(r[arch+"_rva"],8)
                role = r["observed_implementation"]
                if role == "e_notimpl":
                    assert code[:5] == b"\xb8\x01\x40\x00\x80",(arch,r)
                    assert code[5] == (0xc2 if arch == "x86" else 0xc3)
                elif role == "fixed_one":
                    assert code[:5] == b"\xb8\x01\x00\x00\x00", (arch,r)
                elif role == "fixed_s_ok":
                    assert code[:2] == b"\x33\xc0", (arch,r)
            print("PASS: %s exact OEM PE vtable 26 entries, 12 E_NOTIMPL stubs, FilterGraphics literal" % arch)


def check_other(archive, expected, suffix):
    with zipfile.ZipFile(archive) as z:
        members = [s for s in z.namelist() if s.endswith(suffix)]
        if len(members)!=1:
            raise ValueError("Original cross-platform source not unique: "+suffix)
        digest = hashlib.sha256(z.read(members[0])).hexdigest()
        if digest != expected:
            raise ValueError("Unexpected original source SHA256: "+suffix)
        print("PASS: original "+suffix+" SHA256 "+digest)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--windows-zip",type=Path)
    ap.add_argument("--linux-zip",type=Path)
    ap.add_argument("--macos-zip",type=Path)
    ap.add_argument("--check",action="store_true")
    args=ap.parse_args()
    data=json.loads(RECORDS.read_text(encoding="utf-8"))
    expected=generate(data)
    if args.check and INC.read_text(encoding="utf-8") != expected:
        raise SystemExit("ERROR: C initializer differs from original Windows COM vtable inventory")
    if not args.check:
        INC.write_text(expected,encoding="utf-8")
    print("PASS: 26 original x86/x64 vtable entries; generated C initializer in sync")
    if args.windows_zip:
        check_windows(args.windows_zip,data)
    if args.linux_zip:
        check_other(args.linux_zip,
          "bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0",
          "/raster2printer.c")
    if args.macos_zip:
        check_other(args.macos_zip,
          "72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13",
          "/LeitzPrint.bundle/Contents/MacOS/raster2printer")

if __name__=="__main__":
    main()
