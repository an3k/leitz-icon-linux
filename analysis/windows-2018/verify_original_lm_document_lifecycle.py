#!/usr/bin/env python3
"""Recheck EndDocPort/ClosePort against unmodified original Leitz OEM ZIPs.

The Windows binaries are parsed as PE data, never executed. Checks native
instruction bytes for forwarding, SetEvent, SetJobW command 7, list
cleanup and ClosePort. Rechecks Linux 1.0.3 and macOS Classic sources.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA = {
 'win_zip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
 'linux_zip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
 'mac_zip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
 'x86':'4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e',
 'x64':'8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d',
 'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'mac':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13'
}
# Machine instructions independently transcribed from manufacturer x86/x64.
PROOFS = {
 'x86': {
   0x1047:'8b 46 34',        # ClosePort delegate pointer at +0x34
   0x104e:'ff 76 0c',        # underlying handle at +0x0c
   0x1051:'ff d0',           # call stored ClosePort
   0x105f:'e8 fc 09 00 00',  # free/unlink native port
   0x1072:'b8 01 00 00 00',  # return TRUE independent of delegate
   0x13d8:'ff 73 70',        # event +0x70
   0x13db:'ff 15 24 30 00 10', # SetEvent import
   0x13e1:'ff 73 0c',        # underlying port +0x0c
   0x13e4:'8b 43 30',        # EndDoc delegate at +0x30
   0x13e7:'ff d0',           # call original EndDocPort
   0x1400:'6a 07',           # SetJobW command 7
   0x1402:'6a 00',           # SetJobW pJob=NULL
   0x1404:'6a 00',           # SetJobW level=0
   0x140c:'e8 59 07 00 00',  # SetJobW thunk RVA 0x1b6a
   0x1424:'e8 f7 06 00 00',  # clear list RVA 0x1b20
 },
 'x64': {
   0x1106:'48 8b 41 68',       # ClosePort pointer +0x68
   0x1112:'48 8b 49 18',       # underlying handle +0x18
   0x1116:'ff d0',             # call original ClosePort
   0x1128:'e8 03 0c 00 00',    # free/unlink original port RVA 0x1d30
   0x113a:'b8 01 00 00 00',    # TRUE
   0x1612:'48 8b 89 e0 00 00 00',  # event +0xe0
   0x1619:'ff 15 29 1a 00 00',# SetEvent import
   0x161f:'48 8b 4f 18',     # underlying handle
   0x1623:'ff 57 60',        # EndDoc pointer +0x60
   0x1631:'8b 53 18',        # job ID +0x18
   0x1634:'48 8b 4b 10',     # printer handle +0x10
   0x163b:'45 33 c0',        # level 0
   0x163e:'c7 44 24 20 07 00 00 00', # command 7
   0x1646:'e8 71 08 00 00',# SetJobW thunk 0x1ebc
   0x1667:'e8 f4 07 00 00',# free list RVA 0x1e60
 }
}

def sha(raw): return hashlib.sha256(raw).hexdigest()
def check(data,name):
    if sha(data)!=SHA[name]: raise ValueError(f'Original checksum differs: {name}')
def get_one(archive,suffix):
    names=[n for n in archive.namelist() if n.endswith(suffix)]
    if len(names)!=1: raise ValueError(f'Expected exactly one {suffix}; got {len(names)}')
    return archive.read(names[0])

class PE:
    def __init__(self,data):
        self.data=data
        p=struct.unpack_from('<I',data,0x3c)[0]
        if data[p:p+4]!=b'PE\0\0': raise ValueError('Not a Windows PE original')
        count=struct.unpack_from('<H',data,p+6)[0]
        opts=struct.unpack_from('<H',data,p+20)[0]
        self.sections=[struct.unpack_from('<IIII',data,p+24+opts+i*40+8)
                       for i in range(count)]
    def at(self,rva,n):
        for virtual_size,vaddr,raw_size,raw_at in self.sections:
            if vaddr<=rva and rva+n<=vaddr+raw_size:
                k=raw_at+rva-vaddr
                return self.data[k:k+n]
        raise ValueError(f'OEM RVA not mapped: {rva:#x}')

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--windows-zip',type=Path,required=True)
    parser.add_argument('--linux-zip',type=Path)
    parser.add_argument('--macos-zip',type=Path)
    args=parser.parse_args()
    check(args.windows_zip.read_bytes(),'win_zip')
    with zipfile.ZipFile(args.windows_zip) as z:
        for arch,directory in [('x86','i386'),('x64','amd64')]:
            data=get_one(z,'/Drivers/'+directory+'/LeitzLm.dll')
            check(data,arch)
            pe=PE(data)
            for rva,hexcode in PROOFS[arch].items():
                code=bytes.fromhex(hexcode)
                if pe.at(rva,len(code))!=code:
                    raise ValueError(f'{arch} OEM opcode changed: {rva:#x}')
            print(f'PASS: original {arch} EndDocPort/ClosePort {len(PROOFS[arch])} native opcode checks')
    for path,archive_key,file_key,name in [
        (args.linux_zip,'linux_zip','linux','/raster2printer.c'),
        (args.macos_zip,'mac_zip','mac','/LeitzPrint.bundle/Contents/MacOS/raster2printer')]:
        if path:
            check(path.read_bytes(),archive_key)
            with zipfile.ZipFile(path) as z: check(get_one(z,name),file_key)
            print(f'PASS: original {file_key} native raster source')

if __name__=='__main__':main()
