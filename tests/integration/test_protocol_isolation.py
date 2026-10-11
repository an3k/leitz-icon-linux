#!/usr/bin/env python3
"""Post-build module removal/addition: static PPD, no rebuild, no hardware."""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[2]
MODULES=ROOT/"driver/modules"
CLI=ROOT/"leitz-cli"
FILTER=ROOT/"leitz-rastertoicon-mock"
PPD=ROOT/"driver/cups/LeitzIcon.ppd"
ppd_before=hashlib.sha256(PPD.read_bytes()).hexdigest()
assert b"*DefaultLeitzProtocol: Auto" in PPD.read_bytes()
assert b"*LeitzProtocol Native/" in PPD.read_bytes()
assert b"*LeitzProtocol Current/" in PPD.read_bytes()

with tempfile.TemporaryDirectory(prefix="leitz-modules-") as t:
    folder=Path(t)
    pgm=folder/"raster.pgm"
    pgm.write_bytes(b"P5\n16 8\n255\n"+b"\xff"*16*2+b"\x00"*16+b"\xff"*16*5)
    sources={"Current":MODULES/"leitz-current.so","Native":MODULES/"leitz-native.so"}
    for name,src in sources.items():
        assert src.is_file(),name
    paths={name:folder/src.name for name,src in sources.items()}
    for name,src in sources.items():
        shutil.copy2(src,paths[name])
    env=os.environ.copy()
    env["LEITZ_PROTOCOL_DIR"]=str(folder)
    env["MOCK_MEDIA_NAME"]="Custom"

    def job(option,expected):
        opts="PageSize=Custom"+(" LeitzProtocol="+option if option else "")
        proc=subprocess.run([str(FILTER),"1","root","Module test","1",opts,str(pgm)],
                            env=env,capture_output=True,timeout=15)
        assert (proc.returncode==0)==expected,(option,proc.returncode,proc.stderr)
        if expected:
            assert proc.stdout
            spool=folder/"job.bin"
            spool.write_bytes(proc.stdout)
            checked=subprocess.run([str(CLI),"validate-auto",str(spool)],
                                   env=env,capture_output=True,timeout=5)
            assert checked.returncode==0,(option,checked.stderr)
            assert checked.stdout==b"MAX_WIDTH=16\n"
            return proc.stdout
        assert not proc.stdout
        return b""

    original=job("Auto",True)
    assert original==job("Current",True)
    native_before=job("Native",True)
    assert native_before!=original

    paths["Current"].unlink()
    assert job("Auto",True)==native_before
    assert job("",True)==native_before
    job("Current",False)
    assert job("Native",True)==native_before

    paths["Native"].unlink()
    job("Auto",False)
    job("Current",False)
    job("Native",False)

    shutil.copy2(sources["Current"],paths["Current"])
    assert job("Auto",True)==original
    assert job("Current",True)==original
    job("Native",False)

    shutil.copy2(sources["Native"],paths["Native"])
    assert job("Auto",True)==original
    assert job("Native",True)==native_before

assert hashlib.sha256(PPD.read_bytes()).hexdigest()==ppd_before
print("PASS unchanged PPD; Auto priority; two, one, zero, and restored .so modules")
