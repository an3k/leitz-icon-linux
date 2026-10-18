#!/usr/bin/env python3
"""Confirm OEM LeitzRp 2018 GDI media probes from all 3 original archives.

Parses original PE sections directly (no execution of DLLs, no external
packages). Checks 13 selector immediates, CreateDCW/DeleteDC call sites,
and the UTF-16 WINSPOOL literal independently for x86 and x64.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA = {
    'windows_zip': 'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
    'linux_zip': '54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
    'mac_zip': '6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
    'x86': '7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33',
    'x64': '876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787',
    'linux': 'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
    'mac': '72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
SELECTORS = [10,8,10,4,6,88,90,40,42,110,111,112,113]
LOCATIONS = {
    'x86': {
        'selectors': [0x1963,0x1968,0x1973,0x197e,0x1985,0x19c6,0x19d0,
                      0x19fc,0x1a06,0x1a32,0x1a3c,0x1a68,0x1a72],
        'create': (0x18df,bytes.fromhex('ff 15 04 40 00 10')),
        'destroy':(0x1aa6,bytes.fromhex('ff 15 08 40 00 10')),
        'spool_rva':0x456c,
    },
    'x64': {
        'selectors': [0x19a2,0x19b8,0x19c9,0x19da,0x19ef,0x1a2e,0x1a3c,
                      0x1a6d,0x1a7b,0x1aac,0x1aba,0x1aeb,0x1af9],
        'create': (0x192f,bytes.fromhex('ff 15 d3 26 00 00')),
        'destroy':(0x1b2d,bytes.fromhex('ff 15 dd 24 00 00')),
        'spool_rva':0x46d8,
    }
}

def sha(bytes_):
    return hashlib.sha256(bytes_).hexdigest()

def one_file(z, suffix):
    matches=[f for f in z.namelist() if f.endswith(suffix)]
    if len(matches)!=1:
        raise ValueError(f'Expected unique OEM file {suffix}, got {len(matches)}')
    return z.read(matches[0])

class OriginalPE:
    def __init__(self, raw):
        self.raw=raw
        p=struct.unpack_from('<I',raw,0x3c)[0]
        if raw[p:p+4]!=b'PE\0\0':
            raise ValueError('Original Windows PE signature missing')
        count=struct.unpack_from('<H',raw,p+6)[0]
        opt=struct.unpack_from('<H',raw,p+20)[0]
        self.sections=[]
        for i in range(count):
            at=p+24+opt+40*i
            virtual_size,rva,raw_size,raw_start=struct.unpack_from('<IIII',raw,at+8)
            self.sections.append((rva,raw_start,raw_size))

    def read(self,rva,n):
        for start,raw_start,sz in self.sections:
            if start<=rva and rva+n<=start+sz:
                p=raw_start+rva-start
                return self.raw[p:p+n]
        raise ValueError(f'Original PE RVA unmapped: 0x{rva:x}')

def verify_archive(p, key):
    raw=Path(p).read_bytes()
    if sha(raw)!=SHA[key]:
        raise ValueError(f'SHA256 mismatch for {key}')
    return raw

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--windows-zip',type=Path,required=True)
    ap.add_argument('--linux-zip',type=Path)
    ap.add_argument('--macos-zip',type=Path)
    args=ap.parse_args()
    verify_archive(args.windows_zip,'windows_zip')
    with zipfile.ZipFile(args.windows_zip) as z:
        for arch in ('x86','x64'):
            raw=one_file(z,'/Drivers/'+('i386' if arch=='x86' else 'amd64')+'/LeitzRp.dll')
            if sha(raw)!=SHA[arch]:
                raise ValueError(f'{arch} original DLL does not match audited original')
            pe=OriginalPE(raw)
            site=LOCATIONS[arch]
            for offset,selector in zip(site['selectors'], SELECTORS):
                native=(b'\x6a'+bytes([selector])) if arch=='x86' else (
                    b'\xba'+struct.pack('<I',selector))
                if pe.read(offset,len(native))!=native:
                    raise ValueError(f'{arch} GetDeviceCaps selector mismatch at 0x{offset:x}')
            for what in ('create','destroy'):
                loc,code=site[what]
                if pe.read(loc,len(code))!=code:
                    raise ValueError(f'{arch} native {what} call changed')
            if pe.read(site['spool_rva'],len('WINSPOOL\0')*2)!= 'WINSPOOL\0'.encode('utf-16-le'):
                raise ValueError(f'{arch} CreateDCW driver string mismatch')
            print(f'PASS: original Windows {arch} 13 GetDeviceCaps selectors + CreateDCW/DeleteDC + UTF16 WINSPOOL')
    if args.linux_zip:
        verify_archive(args.linux_zip,'linux_zip')
        with zipfile.ZipFile(args.linux_zip) as z:
            if sha(one_file(z,'/raster2printer.c'))!=SHA['linux']:
                raise ValueError('Linux original source changed')
            print('PASS: native Linux 1.0.3 raster2printer.c')
    if args.macos_zip:
        verify_archive(args.macos_zip,'mac_zip')
        with zipfile.ZipFile(args.macos_zip) as z:
            if sha(one_file(z,'/LeitzPrint.bundle/Contents/MacOS/raster2printer'))!=SHA['mac']:
                raise ValueError('macOS Classic native raster filter changed')
            print('PASS: original macOS Classic 5.1.465 raster2printer')

if __name__=='__main__':
    main()
