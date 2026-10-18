#!/usr/bin/env python3
"""Check actual x86/x64 original LeitzRp PaperSize GetDriverSetting calls.

Reads only the three user-supplied original OEM ZIPs (never executes DLLs).
Portable C output is regression-tested independently, not assumed equal
merely from these original machine instructions.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA={
 'wzip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
 'lzip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
 'mzip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
 'x86':'7586da2d722f76991002fce4062c45625b733dc01902484e117c5bf748f5cd33',
 'x64':'876f3fb48829420fad03692d6d613c28f0150d797c928ba45c021e7edf640787',
 'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'mac':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
# Disassembly-derived original RVAs, expected literal opcode bytes.
PROOFS={
 'x86':{
   0x16f8:'68 54 44 00 10',      # PaperSize pointer
   0x1703:'c7 85 a8 ef ff ff 00 00 00 00', # needed=0
   0x171a:'ff 51 0c',           # first OEM host virtual call
   0x176b:'8b 85 a8 ef ff ff',   # first pcbNeeded
   0x1771:'85 c0',              # nonzero gate
   0x177a:'ff 15 5c 40 00 10',  # allocate needed
   0x17a3:'68 54 44 00 10',     # same PaperSize
   0x17b7:'ff 51 0c',           # second OEM host virtual call
   0x186a:'ff 15 58 40 00 10', # wcsstr Continuous
   0x1877:'c7 46 10 00 00 00 00', # IsDieCut=0
   0x1887:'ff 15 58 40 00 10', # wcsstr CUSTOMSIZE
   0x1894:'c7 46 14 01 00 00 00', # IsCustomSize=1
 },
 'x64':{
   0x172e:'4c 8d 05 73 2e 00 00', # PaperSize pointer
   0x1735:'45 33 c9',          # pOutput=NULL
   0x173f:'44 89 74 24 20',    # cbSize=0
   0x1744:'ff 50 18',          # first OEM host call
   0x179b:'85 c0',             # nonzero first pcbNeeded
   0x17a5:'ff 15 15 29 00 00',# allocate first needed
   0x17af:'4c 8d 05 f2 2d 00 00', # same PaperSize
   0x17de:'41 ff 52 18',       # second OEM host call
   0x18c2:'ff 15 d8 27 00 00',# wcsstr Continuous
   0x18cd:'44 89 76 14',       # IsDieCut=0
   0x18dd:'ff 15 bd 27 00 00',# wcsstr CUSTOMSIZE
   0x18e8:'44 89 7e 18',       # IsCustomSize=1
 },
}
RDATA={
 'x86':{0x4454:b'PaperSize\0',0x42d0:'Continuous\0'.encode('utf-16-le'),0x42e8:'CUSTOMSIZE\0'.encode('utf-16-le')},
 'x64':{0x45a8:b'PaperSize\0',0x4408:'Continuous\0'.encode('utf-16-le'),0x4420:'CUSTOMSIZE\0'.encode('utf-16-le')},
}
def sha(v):return hashlib.sha256(v).hexdigest()
def member(z,suffix):
    paths=[n for n in z.namelist() if n.endswith(suffix)]
    if len(paths)!=1:raise ValueError('Original member not unique '+suffix)
    return z.read(paths[0])
class PE:
    def __init__(self,b):
        self.b=b;p=struct.unpack_from('<I',b,0x3c)[0]
        if b[p:p+4]!=b'PE\0\0':raise ValueError('Expected PE')
        sec=struct.unpack_from('<H',b,p+6)[0]
        opt=struct.unpack_from('<H',b,p+20)[0]
        self.sec=[struct.unpack_from('<IIII',b,p+24+opt+i*40+8) for i in range(sec)]
    def read(self,r,n):
        for vs,addr,raw_size,pos in self.sec:
            if addr<=r and r+n<=addr+raw_size:
                return self.b[pos+r-addr:pos+r-addr+n]
        raise ValueError('Original RVA unavailable: '+hex(r))
def main():
    p=argparse.ArgumentParser()
    p.add_argument('--windows-zip',required=True,type=Path)
    p.add_argument('--linux-zip',type=Path)
    p.add_argument('--macos-zip',type=Path)
    a=p.parse_args()
    if sha(a.windows_zip.read_bytes())!=SHA['wzip']:raise ValueError('OEM Windows ZIP SHA256 changed')
    with zipfile.ZipFile(a.windows_zip) as z:
        for arch,sub in [('x86','i386'),('x64','amd64')]:
            dll=member(z,f'/Drivers/{sub}/LeitzRp.dll')
            if sha(dll)!=SHA[arch]:raise ValueError('Original '+arch+' DLL SHA256 changed')
            pe=PE(dll)
            for r,code in PROOFS[arch].items():
                expected=bytes.fromhex(code)
                if pe.read(r,len(expected))!=expected:raise ValueError(f'{arch} opcode mismatch at {r:#x}')
            for r,expected in RDATA[arch].items():
                if pe.read(r,len(expected))!=expected:raise ValueError(f'{arch} OEM string mismatch at {r:#x}')
            print('PASS',arch,len(PROOFS[arch]),'native opcodes plus 3 literal OEM setting strings')
    for path,key,suffix,itemkey in [(a.linux_zip,'lzip','/raster2printer.c','linux'),
        (a.macos_zip,'mzip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac')]:
        if path:
            if sha(path.read_bytes())!=SHA[key]:raise ValueError('Original ZIP checksum mismatch: '+key)
            with zipfile.ZipFile(path) as z:
                if sha(member(z,suffix))!=SHA[itemkey]:raise ValueError('Original source mismatch '+itemkey)
            print('PASS original',itemkey,'native raster source SHA256')
if __name__=='__main__':main()
