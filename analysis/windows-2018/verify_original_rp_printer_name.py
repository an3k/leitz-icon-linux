#!/usr/bin/env python3
"""Verify original 2018 LeitzRp.dll printer-name GetPrinterW evidence.

Read Windows DLLs directly from the uploaded OEM ZIP and cross-check
manufacturer Linux/macOS filter hashes, without executing proprietary code.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA = {
    'windows_zip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
    'linux_zip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
    'mac_zip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
    'x86':'7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33',
    'x64':'876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787',
    'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
    'mac':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
# Independent instruction patterns from actual OEM disassembly.
PROOFS={
 'x86':{
   0x1395:'6a 1c',                 # original 28-byte output-holder allocation
   0x1416:'6a 01',                 # GetPrinterW level 1, first call
   0x141b:'ff 15 c8 40 00 10',     # GetPrinterW via IAT
   0x1421:'8b b5 f8 f7 ff ff',     # read pcbNeeded
   0x1444:'6a 01',                 # GetPrinterW level 1, second call
   0x1449:'ff 15 c8 40 00 10',     # second call, same IAT
   0x144f:'85 c0',                 # gate on second BOOL result
   0x1453:'8b 77 08',              # PRINTER_INFO_1W.pName x86 +8
   0x1460:'66 8b 06',              # wcslen-like UTF16 WORD access
   0x14a2:'ff 77 08',              # copy from pName pointer
 },
 'x64':{
   0x1383:'b9 20 00 00 00',       # original 32-byte output holder
   0x13f9:'41 8d 51 01',          # first query level 1
   0x1405:'ff 15 85 2d 00 00',    # GetPrinterW IAT
   0x140b:'8b 5c 24 30',          # pcbNeeded
   0x1436:'ba 01 00 00 00',       # second query level 1
   0x1443:'ff 15 47 2d 00 00',    # same GetPrinterW IAT
   0x1449:'85 c0',                # second BOOL gate
   0x144d:'48 8b 46 10',          # PRINTER_INFO_1W.pName x64 +16
   0x1463:'66 83 3c 58 00',       # UTF16 termination check
   0x1496:'48 8b 56 10',          # read pName for copy
 }
}

def sha(data):return hashlib.sha256(data).hexdigest()

def unique(z,suffix):
    names=[n for n in z.namelist() if n.endswith(suffix)]
    if len(names)!=1:raise ValueError(f'Expected exactly one OEM entry {suffix}, got {len(names)}')
    return z.read(names[0])

def read_rva(binary,rva,n):
    p=struct.unpack_from('<I',binary,0x3c)[0]
    if binary[p:p+4]!=b'PE\0\0':raise ValueError('Not original PE file')
    sections=struct.unpack_from('<H',binary,p+6)[0]
    optional_size=struct.unpack_from('<H',binary,p+20)[0]
    for i in range(sections):
        o=p+24+optional_size+40*i
        vs,va,size,offset=struct.unpack_from('<IIII',binary,o+8)
        if va<=rva and rva+n<=va+size:
            start=offset+rva-va
            return binary[start:start+n]
    raise ValueError(f'Unmapped RVA 0x{rva:x}')

def source_archive(path,key,suffix,internal_key):
    data=Path(path).read_bytes()
    if sha(data)!=SHA[key]:raise ValueError(f'Original ZIP hash changed: {key}')
    with zipfile.ZipFile(path) as z:original=unique(z,suffix)
    if sha(original)!=SHA[internal_key]:raise ValueError(f'Original source hash changed: {internal_key}')
    print('PASS',key,internal_key)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--windows-zip',type=Path,required=True)
    ap.add_argument('--linux-zip',type=Path)
    ap.add_argument('--macos-zip',type=Path)
    a=ap.parse_args()
    blob=a.windows_zip.read_bytes()
    if sha(blob)!=SHA['windows_zip']:raise ValueError('Windows OEM ZIP does not match uploaded original')
    with zipfile.ZipFile(a.windows_zip) as z:
        for arch,sub in [('x86','i386'),('x64','amd64')]:
            raw=unique(z,f'/Drivers/{sub}/LeitzRp.dll')
            if sha(raw)!=SHA[arch]:raise ValueError(f'{arch} original OEM DLL changed')
            for rva,hex_code in PROOFS[arch].items():
                code=bytes.fromhex(hex_code)
                if read_rva(raw,rva,len(code))!=code:
                    raise ValueError(f'{arch} OEM GetPrinterW opcode mismatch RVA 0x{rva:x}')
            print(f'PASS original {arch} LeitzRp GetPrinterW: {len(PROOFS[arch])} native opcode checks')
    if a.linux_zip:
        source_archive(a.linux_zip,'linux_zip','/raster2printer.c','linux')
    if a.macos_zip:
        source_archive(a.macos_zip,'mac_zip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac')
if __name__=='__main__':main()
