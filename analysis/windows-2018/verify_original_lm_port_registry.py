#!/usr/bin/env python3
"""Read original Windows LeitzLm x86/x64; verify native port registry helpers.

No proprietary binary execution. Optional checks of original Linux/macOS ZIPs.
"""
import argparse
import hashlib
import struct
import zipfile
from pathlib import Path

EXPECTED={
 'windows_zip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
 'linux_zip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
 'macos_zip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
 'i386':'4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e',
 'amd64':'8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d',
 'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'macos':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13'
}
# Native bytes at original RVAs confirmed from unmodified OEM LeitzLm.dll.
PATTERNS={
'i386':{
0x1ae4:'8b 35 84 40 00 10', # global list head
0x1aee:'85 ff',             # NULL search
0x1af2:'66 83 3f 00',       # empty UTF16 search
0x1afd:'8b 1d 44 30 00 10', # lstrcmpiW IAT +0x3044
0x1b03:'ff 36',             # existing port's name
0x1b06:'ff d3',             # case-insensitive comparison
0x1b0c:'8b 76 04',          # next pointer +4
0x19b5:'68 78 04 00 00',    # native allocation 0x478
0x19c9:'68 78 04 00 00',    # memset full allocation
0x19d8:'e8 73 ff ff ff',    # UTF16 name copy helper
0x19e8:'89 37',             # store name pointer
0x19f3:'89 47 04',          # original next link +4
0x19f6:'89 3d 84 40 00 10', # insert at global head
0x19fe:'c7 47 10 00 00 00 00', # printer handle NULL
0x1a05:'c7 47 08 00 00 00 00', # jobs NULL
0x1a0c:'c6 47 6c 00',     # state byte zero
0x1a10:'ff 15 48 30 00 10', # CreateEventW(NULL,TRUE,TRUE,NULL)
0x1a16:'68 00 04 00 00',    # 0x400 scratch
0x1a1b:'89 47 70',          # event at +0x70
0x1a1e:'8d 47 74',          # scratch at +0x74
0x1a24:'e8 f9 04 00 00',    # scratch memset thunk
},
'amd64':{
0x1e0a:'48 8b 1d d7 22 00 00', # global head
0x1e14:'48 85 c9',          # null input
0x1e19:'66 83 39 00',       # empty name
0x1e24:'48 8b 13',          # existing name
0x1e2a:'ff 15 58 12 00 00',# lstrcmpiW IAT 0x3088
0x1e34:'48 8b 5b 08',       # next ptr +8
0x1c52:'ba f0 04 00 00',    # allocate 0x4f0
0x1c6f:'41 b8 f0 04 00 00', # zero 0x4f0
0x1c80:'e8 3b ff ff ff',    # copy name
0x1c8d:'48 89 03',          # store name
0x1c9c:'48 89 43 08',       # set next
0x1ca0:'48 89 1d 41 24 00 00', # insert at list head
0x1caf:'48 c7 43 20 00 00 00 00', # printer NULL
0x1cb7:'48 c7 43 10 00 00 00 00', # jobs NULL
0x1cbf:'c6 83 d8 00 00 00 00', # flag zero
0x1cc6:'ff 15 c4 13 00 00',# CreateEventW
0x1ccc:'48 8d 8b e8 00 00 00', # scratch +0xe8
0x1cd5:'41 b8 00 04 00 00',# 1024 scratch bytes
0x1cdb:'48 89 83 e0 00 00 00', # event +0xe0
0x1ce2:'e8 bf 06 00 00',   # memset scratch
}
}
def digest(data):return hashlib.sha256(data).hexdigest()
def one(z,suffix):
    paths=[n for n in z.namelist() if n.endswith(suffix)]
    if len(paths)!=1:raise ValueError(f'Expected unique {suffix}; got {len(paths)}')
    return z.read(paths[0])
class PE:
    def __init__(self,raw):
        self.raw=raw;p=struct.unpack_from('<I',raw,0x3c)[0]
        if raw[p:p+4]!=b'PE\0\0': raise ValueError('Invalid original OEM PE')
        n=struct.unpack_from('<H',raw,p+6)[0]; sz=struct.unpack_from('<H',raw,p+20)[0]
        self.sections=[struct.unpack_from('<IIII',raw,p+24+sz+40*i+8) for i in range(n)]
    def at(self,rva,n):
        for vs,va,size,offset in self.sections:
            if va<=rva and rva+n<=va+size:
                return self.raw[offset+rva-va:offset+rva-va+n]
        raise ValueError(f'Unmapped original RVA {rva:#x}')
def check_zip(path,key,suffix,internal):
    if digest(path.read_bytes())!=EXPECTED[key]:raise ValueError(f'{key} archive digest mismatch')
    with zipfile.ZipFile(path) as z: content=one(z,suffix)
    if digest(content)!=EXPECTED[internal]:raise ValueError(f'{internal} file digest mismatch')
    print('PASS',key,internal)
def main():
    a=argparse.ArgumentParser()
    a.add_argument('--windows-zip',required=True,type=Path)
    a.add_argument('--linux-zip',type=Path)
    a.add_argument('--macos-zip',type=Path)
    options=a.parse_args()
    if digest(options.windows_zip.read_bytes())!=EXPECTED['windows_zip']:
        raise ValueError('Original Windows archive changed')
    with zipfile.ZipFile(options.windows_zip) as z:
        for arch in ('i386','amd64'):
            raw=one(z,f'/Drivers/{arch}/LeitzLm.dll')
            if digest(raw)!=EXPECTED[arch]: raise ValueError(f'Original {arch} DLL changed')
            if b'lstrcmpiW\0' not in raw or b'CreateEventW\0' not in raw:
                raise ValueError(f'Missing imported Windows APIs: {arch}')
            pe=PE(raw)
            for rva,hexpattern in PATTERNS[arch].items():
                expected=bytes.fromhex(hexpattern)
                if pe.at(rva,len(expected))!=expected:
                    raise ValueError(f'{arch} original mismatch at {rva:#x}')
            print(f'PASS original {arch}: {len(PATTERNS[arch])} native port registry opcode checks')
    if options.linux_zip:check_zip(options.linux_zip,'linux_zip','/raster2printer.c','linux')
    if options.macos_zip:check_zip(options.macos_zip,'macos_zip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','macos')
if __name__=='__main__':main()
