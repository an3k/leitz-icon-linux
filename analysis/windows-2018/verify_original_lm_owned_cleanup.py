#!/usr/bin/env python3
"""Prove original LeitzLm Shutdown, port destructor and DeletePrinter import.

Only reads original Windows/Linux/macOS manufacturer ZIPs; no OEM binaries
are executed or distributed. The destructive DeletePrinter operation is
represented by a callable boundary in C, never actually performed here.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zipfile

SHA={
 'windows_zip':'cc6e175fb3f183d211cd1b4610ca2d096ec958421bc45b4bc8fc2740be3034ca',
 'linux_zip':'54e1a92b830e1281975ca8b1f7018275327fa8f214f8bb440fda6e83d80abe17',
 'mac_zip':'6a3d6cdd617947dcb33b1d58dd7dc2fc1427601091a8ff7079a013fb7ffa4210',
 'x86':'4faeba4f90200c3cc56523bc6e4caa4784d384766fadc8f38378153c0e89947e',
 'x64':'8d58d21fdfedae812863a4efae30c6132a196d563c3e3a2f018667d3bd29561d',
 'linux':'bc06c5a2c590ff242f0f0c31e6ba052530ec0b6666c4c673d1d89f2a95dc5cd0',
 'mac':'72f1dfc66f755f9537330c1f0306828f4510ba3b44296a192d1ee8a2dec12c13',
}
# Instruction byte patterns independently observed from both manufacturer PE files.
# Absolute addresses in objdump are IMAGEBASE+RVA; stored here as RVA.
PROOFS={
 'x86': {
  0x1000:'55 8b ec 57',       # Shutdown entry
  0x1012:'8b 47 08',          # current port head at monitor+8
  0x1020:'8b 70 04',          # save next port before freeing current
  0x1024:'e8 37 0a 00 00',    # original shared helper 0x1a60
  0x1033:'57',                # free monitor allocation
  0x1034:'ff 15 04 30 00 10',# GlobalFree import
  0x1a63:'8b 0d 84 40 00 10',# current global port head
  0x1a71:'8b 47 04',          # next pointer port+4
  0x1a8f:'8b 47 04',          # unlink middle/tail
  0x1a95:'8b 47 10',          # printer resource at port+16
  0x1a9d:'e8 ce 00 00 00',    # call DeletePrinter thunk 0x1b70
  0x1aa2:'8b 47 70',          # event handle at port+0x70
  0x1aaa:'ff 15 40 30 00 10',# CloseHandle event
  0x1ab0:'57',                # call job cleanup
  0x1ab1:'e8 6a 00 00 00',    # job cleanup 0x1b20
  0x1ab9:'ff 37',             # free name at port+0
  0x1ac2:'c7 07 00 00 00 00',# zero name field before free port
  0x1b32:'8b 43 08',          # first job at port+8
  0x1b53:'c7 43 08 00 00 00 00',# clear jobs under lock
  0x1b70:'ff 25 cc 30 00 10', # native WINSPOOL DeletePrinter IAT thunk
 },
 'x64': {
  0x10b0:'40 57',                 # Shutdown entry
  0x10c6:'48 8b 4f 10',           # head monitor+16
  0x10d4:'48 8b 59 08',           # saved next port+8
  0x10d8:'e8 53 0c 00 00',        # shared 0x1d30 cleanup
  0x10f2:'48 ff 25 0f 1f 00 00', # GlobalFree monitor
  0x1d3a:'48 8b 15 a7 23 00 00', # global port list
  0x1d49:'48 8b 41 08',          # unlink first
  0x1d73:'48 8b 41 08',          # unlink any middle node
  0x1d7b:'48 8b 49 20',          # printer resource port+32
  0x1d84:'e8 39 01 00 00',       # DeletePrinter thunk
  0x1d89:'48 8b 8f e0 00 00 00', # event handle+0xe0
  0x1d95:'ff 15 e5 12 00 00',   # CloseHandle
  0x1da8:'48 8b 4f 10',          # job list head+16
  0x1db1:'48 8b 19',             # job->next
  0x1db4:'ff 15 4e 12 00 00',   # GlobalFree job
  0x1dcb:'48 89 5f 10',          # clear job head
  0x1dd5:'48 8b 0f',             # native port name pointer
  0x1dd8:'ff 15 2a 12 00 00',   # GlobalFree name
  0x1de1:'48 89 1f',             # reset name before final free
  0x1dee:'48 ff 25 13 12 00 00',# GlobalFree port
  0x1ec2:'ff 25 d8 12 00 00',   # imported WINSPOOL DeletePrinter thunk
 }
}
def sha(data): return hashlib.sha256(data).hexdigest()
def check(data, identifier):
    if sha(data)!=SHA[identifier]:
        raise ValueError('Manufacturer original SHA mismatch: '+identifier)
def unique(z,suffix):
    names=[n for n in z.namelist() if n.endswith(suffix)]
    if len(names)!=1:raise ValueError('Expected exactly one ZIP member ending '+suffix)
    return z.read(names[0])
class PE:
    def __init__(self,raw):
        self.raw=raw;p=struct.unpack_from('<I',raw,0x3c)[0]
        if raw[p:p+4]!=b'PE\0\0':raise ValueError('Not a PE file')
        n=struct.unpack_from('<H',raw,p+6)[0]; opt=struct.unpack_from('<H',raw,p+20)[0]
        self.sections=[struct.unpack_from('<IIII',raw,p+24+opt+i*40+8) for i in range(n)]
    def read(self,rva,n):
        for vs,addr,raw_size,at in self.sections:
            if addr<=rva and rva+n<=addr+raw_size:
                return self.raw[at+rva-addr:at+rva-addr+n]
        raise ValueError('Original RVA not mapped '+hex(rva))
def main():
    p=argparse.ArgumentParser()
    p.add_argument('--windows-zip',type=Path,required=True)
    p.add_argument('--linux-zip',type=Path)
    p.add_argument('--macos-zip',type=Path)
    a=p.parse_args()
    check(a.windows_zip.read_bytes(),'windows_zip')
    with zipfile.ZipFile(a.windows_zip) as z:
        for arch,dirname in [('x86','i386'),('x64','amd64')]:
            raw=unique(z,f'/Drivers/{dirname}/LeitzLm.dll');check(raw,arch)
            pe=PE(raw)
            for rva,code in PROOFS[arch].items():
                expected=bytes.fromhex(code)
                if pe.read(rva,len(expected)) != expected:
                    raise ValueError(f'Original {arch} instruction changed at {rva:#x}')
            if b'DeletePrinter\0' not in raw or b'WINSPOOL.DRV\0' not in raw:
                raise ValueError(f'No OEM DeletePrinter import evidence in {arch}')
            print(f'PASS original {arch} LeitzLm port release/Shutdown: {len(PROOFS[arch])} instructions and DeletePrinter import')
    for file,zipkey,end,key in [(a.linux_zip,'linux_zip','/raster2printer.c','linux'),
                                (a.macos_zip,'mac_zip','/LeitzPrint.bundle/Contents/MacOS/raster2printer','mac')]:
        if file:
            check(file.read_bytes(),zipkey)
            with zipfile.ZipFile(file) as z: check(unique(z,end),key)
            print('PASS original manufacturer',key,'raster file SHA256')
if __name__=='__main__': main()
