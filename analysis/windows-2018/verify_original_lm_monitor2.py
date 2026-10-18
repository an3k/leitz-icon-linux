#!/usr/bin/env python3
"""Verify Windows LeitzLm MONITOR2 vtables and forwarding from original OEM ZIP.

Reads exact manufacturer ZIP/PE bytes; never runs manufacturer DLLs and
requires no nonstandard Python modules. Optionally cross-checks original
Linux and macOS raster filters for unchanged source provenance.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

EXPECTED = {
    'windows_zip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
    'linux_zip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
    'macos_zip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
    'x86':'4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e',
    'x64':'8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d',
    'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
    'macos':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
TABLE = {
    'x86': [0,0,0,0x11a0,0x12e0,0x1150,0x1080,0x13d0,0x1040,
            0,0,0,0,0x1440,0,0,0,0,0x1000,0,0,0],
    'x64': [0,0,0,0x12a0,0x14b0,0x1220,0x1150,0x1600,0x1100,
            0,0,0,0,0x1000,0,0,0,0,0x10b0,0,0,0],
}
PROOFS = {
    'x86': {
        0x1567:'6a 0c', # InitializePrintMonitor2 allocates 12 bytes
        0x1569:'c7 06 00 00 00 00', # initial output handle = NULL
        0x158e:'89 41 04', # copy registry to native monitor+4
        0x1591:'c7 41 08 00 00 00 00', # linked list head +8 = NULL
        0x15a5:'b8 00 40 00 10', # return original callback table
        0x1161:'85 c0', # WritePort: test monitor port pointer
        0x1170:'6a 57', # 87 ERROR_INVALID_PARAMETER
        0x1187:'ff 70 0c', # stored underlying port handle +0x0c
        0x118a:'8b 40 28', # stored host WritePort method +0x28
        0x118d:'ff d0', # delegate and return its BOOL unmodified
        0x1129:'ff 70 0c', # original ReadPort underlying port handle
        0x112c:'8b 40 2c', # original host ReadPort method +0x2c
        0x112f:'ff d0',
    },
    'x64': {
        0x16ed:'b9 18 00 00 00', # InitializePrintMonitor2 24 bytes
        0x16f5:'48 c7 02 00 00 00 00', # initial output handle = NULL
        0x171b:'48 89 78 08', # copy registry at native monitor+8
        0x171f:'48 c7 40 10 00 00 00 00', # head +0x10 = NULL
        0x173c:'48 8d 05 bd 28 00 00', # return original callback table
        0x124d:'48 85 db', # WritePort: test monitor port pointer
        0x125f:'8d 4b 57', # 87 ERROR_INVALID_PARAMETER
        0x126c:'48 8b 4b 18', # host port handle +0x18
        0x1279:'ff 53 50', # host WritePort +0x50
        0x11ec:'49 8b 4e 18', # read underlying port handle +0x18
        0x11f9:'41 ff 56 58', # host ReadPort +0x58
    },
}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def check(data,expected_key):
    if digest(data)!=EXPECTED[expected_key]:
        raise ValueError('Original source SHA-256 changed: '+expected_key)

def single(z, suffix):
    matches=[n for n in z.namelist() if n.endswith(suffix)]
    if len(matches)!=1:
        raise ValueError(f'Expected one {suffix}, got {len(matches)}')
    return z.read(matches[0])

class PE:
    def __init__(self,blob):
        self.blob=blob
        p=struct.unpack_from('<I',blob,0x3c)[0]
        if blob[p:p+4]!=b'PE\0\0':
            raise ValueError('OEM PE signature mismatch')
        opt=p+24
        self.magic=struct.unpack_from('<H',blob,opt)[0]
        self.pointer_size=8 if self.magic==0x20b else 4 if self.magic==0x10b else 0
        if not self.pointer_size: raise ValueError('Unsupported original PE architecture')
        self.image_base=struct.unpack_from('<Q' if self.pointer_size==8 else '<I',
                          blob,opt+(24 if self.pointer_size==8 else 28))[0]
        count=struct.unpack_from('<H',blob,p+6)[0]
        optional_size=struct.unpack_from('<H',blob,p+20)[0]
        self.sections=[]
        for i in range(count):
            offset=opt+optional_size+40*i
            vs,rva,raw_size,position=struct.unpack_from('<IIII',blob,offset+8)
            self.sections.append((rva,raw_size,position))

    def at(self,rva,length):
        for base,size,position in self.sections:
            if base<=rva and rva+length<=base+size:
                p=position+rva-base
                return self.blob[p:p+length]
        raise ValueError(f'OEM RVA 0x{rva:x} not mapped')

    def monitor_table(self):
        raw=self.at(0x4000,self.pointer_size*22)
        elements=struct.unpack('<'+('Q' if self.pointer_size==8 else 'I')*22,raw)
        return (elements[0],[v-self.image_base if v else 0 for v in elements[1:]])

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--windows-zip',type=Path,required=True)
    parser.add_argument('--linux-zip',type=Path)
    parser.add_argument('--macos-zip',type=Path)
    args=parser.parse_args()
    check(args.windows_zip.read_bytes(),'windows_zip')
    with zipfile.ZipFile(args.windows_zip) as z:
        for arch,sub in (('x86','i386'),('x64','amd64')):
            raw=single(z,f'/Drivers/{sub}/LeitzLm.dll')
            check(raw,arch)
            pe=PE(raw)
            assert pe.pointer_size == (4 if arch=='x86' else 8)
            size,rv=pe.monitor_table()
            assert size == (0x58 if arch=='x86' else 0xb0)
            if [0,*rv] != TABLE[arch]:
                raise ValueError(f'Original {arch} MONITOR2 22-slot table changed: {rv}')
            for rva,hex_pattern in PROOFS[arch].items():
                code=bytes.fromhex(hex_pattern)
                if pe.at(rva,len(code))!=code:
                    raise ValueError(f'{arch} native instruction mismatch at RVA 0x{rva:x}')
            print(f'PASS: original {arch} MONITOR2 table 22 slots + {len(PROOFS[arch])} native instruction signatures')
    for path,zipkind,sourcekind,ending in (
        (args.linux_zip,'linux_zip','linux','/raster2printer.c'),
        (args.macos_zip,'macos_zip','macos','/LeitzPrint.bundle/Contents/MacOS/raster2printer')):
        if path:
            check(path.read_bytes(),zipkind)
            with zipfile.ZipFile(path) as z:check(single(z,ending),sourcekind)
            print(f'PASS: original {sourcekind} filter SHA-256')

if __name__=='__main__':
    main()
