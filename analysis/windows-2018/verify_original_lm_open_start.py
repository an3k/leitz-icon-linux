#!/usr/bin/env python3
"""Check 2018 LeitzLm OpenPortEx / StartDocPort in actual OEM archives.

No binary execution, network or third-party packages. The uploaded archives
are identified by complete SHA256 before checking original PE RVA bytes.
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
 'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'mac':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
# Exact instruction sequences from OEM i386/amd64 LeitzLm.dll PE disassembly.
PROOFS={
 'x86':{
  0x11c9:'8b 75 1c',        # host monitor callback table
  0x11d4:'83 7e 08 00',     # first required function at +8
  0x11de:'83 7e 10 00',     # +16
  0x11e8:'83 7e 14 00',     # +20
  0x11f2:'83 7e 18 00',     # +24
  0x11fc:'83 7e 20 00',     # +32
  0x1221:'68 aa 00 00 00', # ERROR_BUSY/170 for duplicate
  0x125d:'8d 47 0c',       # underlying port handle +12
  0x1264:'8b 46 08',       # host OpenPort function +8
  0x1270:'8b 45 18',       # caller's output port pointer
  0x1288:'f3 a5',          # copy 0x58 native MONITOR2 host bytes
  0x12c9:'68 bf 0b 00 00', # invalid monitor callback set -> 3007
  0x12f0:'83 7d 18 00',    # invalid DOC_INFO fifth argument
  0x1305:'66 83 3b 00',    # empty printer name
  0x1310:'68 60 ea 00 00', # WaitForSingleObject timeout 60000
  0x1318:'ff 15 2c 30 00 10', # WaitForSingleObject
  0x1321:'ff 15 28 30 00 10', # ResetEvent
  0x132e:'8d 04 45 12 00 00 00', # 2 * wcslen + 0x12
  0x1339:'e8 e2 05 00 00',# original job allocator
  0x1364:'e8 fb 07 00 00',# OpenPrinterW imported thunk
  0x1375:'89 47 0c',       # native job id at +12
  0x138c:'89 7e 08',       # head of native job list
  0x13a2:'ff 76 0c',       # native underlying port
  0x13a5:'ff d0',          # host StartDocPort
  0x13b7:'6a 57',          # SetLastError 87
 },
 'x64':{
  0x12fa:'48 39 7b 10',   # required host function +16
  0x1304:'48 39 7b 20',   # +32
  0x130e:'48 39 7b 28',   # +40
  0x1318:'48 39 7b 30',   # +48
  0x1322:'48 39 7b 40',   # +64
  0x1346:'b9 aa 00 00 00',# duplicate -> 170
  0x1384:'4c 8d 47 18',   # underlying port +24
  0x138e:'ff 53 10',      # delegate OpenPort +16
  0x139e:'48 8b c3',      # snapshot native callback table
  0x1420:'0f 28 43 20',   # final 16-byte copy blocks
  0x147a:'b9 bf 0b 00 00',# invalid callback table -> 3007
  0x14d8:'4c 8b b4 24 80 00 00 00', # doc info fifth param
  0x14f2:'66 39 2a',      # empty UTF-16 printer name
  0x1502:'ba 60 ea 00 00',# 60000ms
  0x1511:'ff 15 41 1b 00 00', # WaitForSingleObject
  0x151e:'ff 15 2c 1b 00 00', # ResetEvent
  0x152d:'44 8d 24 45 22 00 00 00', # 2*wcslen+0x22
  0x1538:'e8 43 06 00 00', # job allocator
  0x1567:'e8 4a 09 00 00', # OpenPrinterW thunk
  0x1577:'44 89 7f 18',    # native job id+24
  0x1594:'48 89 7b 10',    # native job list head +16
  0x159e:'48 8b 4b 18',    # underlying port handle+24
  0x15b0:'ff 53 48',       # host StartDocPort +72
  0x15d3:'b9 57 00 00 00',# SetLastError 87
 },
}

def sha(data):return hashlib.sha256(data).hexdigest()
def one(z,suffix):
    names=[n for n in z.namelist() if n.endswith(suffix)]
    if len(names)!=1:raise ValueError(f'Expected unique OEM member {suffix}, found {len(names)}')
    return z.read(names[0])
class PE:
    def __init__(self,data):
        self.data=data
        pe=struct.unpack_from('<I',data,0x3c)[0]
        if data[pe:pe+4]!=b'PE\0\0':raise ValueError('Not OEM PE')
        num=struct.unpack_from('<H',data,pe+6)[0]
        op=struct.unpack_from('<H',data,pe+20)[0]
        self.sections=[struct.unpack_from('<IIII',data,pe+24+op+40*i+8) for i in range(num)]
    def at(self,rva,n):
        for vs,va,sz,offset in self.sections:
            if va<=rva and rva+n<=va+sz:
                return self.data[offset+rva-va:offset+rva-va+n]
        raise ValueError(f'RVA {rva:#x} not mapped in original')
def check_zip(path,key,suffix,inner_key):
    blob=path.read_bytes()
    if sha(blob)!=SHA[key]:raise ValueError(f'Original archive changed: {key}')
    with zipfile.ZipFile(path) as z:inner=one(z,suffix)
    if sha(inner)!=SHA[inner_key]:raise ValueError(f'Original source changed: {inner_key}')
    print('PASS:',key,'SHA256 and original raster filter')
def main():
    p=argparse.ArgumentParser()
    p.add_argument('--windows-zip',type=Path,required=True)
    p.add_argument('--linux-zip',type=Path)
    p.add_argument('--macos-zip',type=Path)
    a=p.parse_args()
    blob=a.windows_zip.read_bytes()
    if sha(blob)!=SHA['windows_zip']:raise ValueError('Windows archive changed')
    with zipfile.ZipFile(a.windows_zip) as z:
        for arch,dirname in [('x86','i386'),('x64','amd64')]:
            blob=one(z,f'/Drivers/{dirname}/LeitzLm.dll')
            if sha(blob)!=SHA[arch]:raise ValueError(f'Original {arch} DLL changed')
            pe=PE(blob)
            for rva,instr in PROOFS[arch].items():
                pattern=bytes.fromhex(instr)
                if pe.at(rva,len(pattern))!=pattern:
                    raise ValueError(f'{arch} original mismatch at RVA {rva:#x}')
            print(f'PASS: original Windows {arch} OpenPortEx/StartDocPort, {len(PROOFS[arch])} machine instruction sequences')
    if a.linux_zip:check_zip(a.linux_zip,'linux_zip','/raster2printer.c','linux')
    if a.macos_zip:check_zip(a.macos_zip,'mac_zip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac')
if __name__=='__main__':main()
