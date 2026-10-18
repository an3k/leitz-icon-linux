#!/usr/bin/env python3
"""Verify LeitzRp OEM raster vtable slots 4/6 against original 2018 PE binaries.

Inputs are original user-provided manufacturer ZIPs, never executed.
No guesswork about the native OEMDMPARAM ABI; only observed imported
hrOEMDevMode call and cached-host Release instructions are asserted.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

ORIGINAL = {
    'linuxzip': '54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
    'winzip': 'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
    'maczip': '6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
    'linux': 'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
    'mac': '72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
    'rp_i386': '7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33',
    'rp_amd64': '876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787',
    'dev_i386': 'fcb83126f09d7c51dcccc78cef6869c0d0fdc60f7b779b51304cf00e95ea5c99',
    'dev_amd64': 'b5bd1b6c0c4a94bcc8747dd3941283537d353517f3a445a8397130f06def5342',
}
PROOFS = {
    'i386': {
        0x15d3:'ff 75 10', 0x15d6:'ff 75 0c',
        0x15d9:'ff 15 48 40 00 10',
        0x15e3:'c2 0c 00',
        0x1347:'8b 4e 08', 0x134a:'85 c9',
        0x1351:'ff 50 08',0x1354:'c7 46 08 00 00 00 00',
        0x135b:'33 c0',
    },
    'amd64': {
        0x15f0:'8b ca', 0x15f2:'49 8b d0',
        0x15f5:'48 ff 25 84 2a 00 00',
        0x1329:'48 8b 49 10', 0x132d:'48 85 c9',
        0x1335:'ff 50 10',0x1338:'48 c7 43 10 00 00 00 00',
        0x1340:'33 c0',
    }
}

def digest(b): return hashlib.sha256(b).hexdigest()
def verify(b, key):
    if digest(b) != ORIGINAL[key]:
        raise ValueError('Original manufacturer SHA mismatch: '+key)

def original(z, suffix):
    matches=[x for x in z.namelist() if x.endswith(suffix)]
    if len(matches)!=1:
        raise ValueError('Expected unique ZIP member ending '+suffix)
    return z.read(matches[0])

def sections(raw):
    off=struct.unpack_from('<I',raw,0x3c)[0]
    if raw[off:off+4]!=b'PE\0\0':
        raise ValueError('Original PE signature missing')
    count=struct.unpack_from('<H',raw,off+6)[0]
    optsize=struct.unpack_from('<H',raw,off+20)[0]
    return [struct.unpack_from('<IIII',raw,off+24+optsize+i*40+8)
            for i in range(count)]

def read_rva(raw, layout, rva, n):
    for vs, addr, raw_size, start in layout:
        if addr <= rva and rva+n <= addr+raw_size:
            return raw[start+rva-addr:start+rva-addr+n]
    raise ValueError('RVA not within original PE raw section: '+hex(rva))

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--windows-zip',type=Path,required=True)
    p.add_argument('--linux-zip',type=Path)
    p.add_argument('--macos-zip',type=Path)
    args=p.parse_args()
    verify(args.windows_zip.read_bytes(),'winzip')
    with zipfile.ZipFile(args.windows_zip) as z:
        for arch in ('i386','amd64'):
            dll=original(z,'/Drivers/'+arch+'/LeitzRp.dll')
            dev=original(z,'/Drivers/'+arch+'/LeitzDevMode.dll')
            verify(dll,'rp_'+arch)
            verify(dev,'dev_'+arch)
            if b'hrOEMDevMode\0' not in dll or b'hrOEMDevMode\0' not in dev:
                raise ValueError('OEM driver import/export missing: '+arch)
            layout=sections(dll)
            for rva,hexcode in PROOFS[arch].items():
                code=bytes.fromhex(hexcode)
                if read_rva(dll,layout,rva,len(code))!=code:
                    raise ValueError('%s opcode differs at 0x%x'%(arch,rva))
            print('PASS: original',arch,'raster COM method 4/6:',len(PROOFS[arch]),'native patterns, original DEVMODE DLL')
    for path,key,suffix,content_key in (
        (args.linux_zip,'linuxzip','/raster2printer.c','linux'),
        (args.macos_zip,'maczip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac')):
        if path:
            verify(path.read_bytes(),key)
            with zipfile.ZipFile(path) as z:
                verify(original(z,suffix),content_key)
            print('PASS:',key,'original native raster source')

if __name__=='__main__':
    main()
