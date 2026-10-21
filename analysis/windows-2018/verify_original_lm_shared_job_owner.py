#!/usr/bin/env python3
"""Verify the shared native port job-list field in untouched Leitz OEM ZIPs.

Windows 2018 LeitzLm.dll's StartDocPort, EndDocPort, job-list clearer and
owned-port destructor access one list at x86 +8 and x64 +16. Parse and
SHA-check manufacturer archives; never execute native DLLs or touch printers.
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
 'x86':'4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e',
 'x64':'8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d',
 'linux_filter':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'mac_filter':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
# Original RVAs and raw opcodes, not relabeled portable C structure offsets.
PROOFS = {
 'x86':{
  0x138c:'89 7e 08',                 # StartDocPort stores job head +8
  0x13e9:'8b 73 08',                 # EndDocPort reads same head +8
  0x1424:'e8 f7 06 00 00',          # EndDocPort calls list clear at RVA 0x1b20
  0x1b32:'8b 43 08',                 # clear helper reads same head +8
  0x1b53:'c7 43 08 00 00 00 00',    # clear helper resets same head +8
 },
 'x64':{
  0x1594:'48 89 7b 10',             # StartDocPort stores head +16
  0x1626:'48 8b 5f 10',             # EndDocPort reads head +16
  0x1667:'e8 f4 07 00 00',          # EndDocPort calls list clear at RVA 0x1e60
  0x1e76:'48 8b 4f 10',             # clear helper reads head +16
  0x1ea1:'48 c7 47 10 00 00 00 00',# clear helper resets head +16
  0x1da8:'48 8b 4f 10',             # owned-port destructor also reads +16
  0x1dcb:'48 89 5f 10',             # owned destructor also resets +16
 },
}

def sha(data):
    return hashlib.sha256(data).hexdigest()

def single_member(z, suffix):
    members=[p for p in z.namelist() if p.endswith(suffix)]
    if len(members)!=1:
        raise ValueError(f'Expected unique manufacturer member {suffix}: {len(members)}')
    return z.read(members[0])

class PE:
    def __init__(self,data):
        self.data=data
        pe=struct.unpack_from('<I',data,0x3c)[0]
        if data[pe:pe+4]!=b'PE\0\0': raise ValueError('Not original Windows PE')
        n=struct.unpack_from('<H',data,pe+6)[0]
        opt=struct.unpack_from('<H',data,pe+20)[0]
        self.sections=[struct.unpack_from('<IIII',data,pe+24+opt+40*i+8) for i in range(n)]
    def read(self,rva,n):
        for _,vaddr,raw_size,raw_ptr in self.sections:
            if vaddr<=rva and rva+n<=vaddr+raw_size:
                start=raw_ptr+rva-vaddr
                return self.data[start:start+n]
        raise ValueError(f'Original section RVA {rva:#x} unmapped')

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--windows-zip',type=Path,required=True)
    ap.add_argument('--linux-zip',type=Path,required=True)
    ap.add_argument('--macos-zip',type=Path,required=True)
    args=ap.parse_args()
    for path,key in ((args.windows_zip,'windows_zip'),(args.linux_zip,'linux_zip'),(args.macos_zip,'mac_zip')):
        if sha(path.read_bytes())!=SHA[key]:raise ValueError('Manufacturer archive changed: '+key)
    with zipfile.ZipFile(args.windows_zip) as z:
        for arch,sub in (('x86','i386'),('x64','amd64')):
            binary=single_member(z,f'/Drivers/{sub}/LeitzLm.dll')
            if sha(binary)!=SHA[arch]:raise ValueError('Original Windows binary changed: '+arch)
            pe=PE(binary)
            for rva,raw in PROOFS[arch].items():
                b=bytes.fromhex(raw)
                if pe.read(rva,len(b))!=b:
                    raise ValueError(f'Original {arch} job-list opcode differs at {rva:#x}')
            print(f'PASS {arch}: {len(PROOFS[arch])} shared job-list opcodes')
    for path,suffix,key in ((args.linux_zip,'/raster2printer.c','linux_filter'),
            (args.macos_zip,'/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac_filter')):
        with zipfile.ZipFile(path) as z: b=single_member(z,suffix)
        if sha(b)!=SHA[key]:raise ValueError('Original native filter changed: '+key)
        print('PASS original',key)

if __name__=='__main__':main()
