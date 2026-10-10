#!/usr/bin/env python3
"""CLI policy tests for experimental single-shot query; no USB access."""
import subprocess
from pathlib import Path

exe = Path(__file__).resolve().parents[1] / "leitz-probe"

def must_reject(*args):
    p = subprocess.run([str(exe), *args], capture_output=True,
                       text=True, check=False, timeout=3)
    assert p.returncode == 2, (args, p.returncode, p.stderr)
    assert "COMMAND=ESC" not in p.stdout, args

must_reject()
must_reject("--device", "/dev/usb/lp0", "--command", "status")
must_reject("--idle-confirmed", "--device", "/dev/null", "--command", "status")
must_reject("--idle-confirmed", "--device", "/dev/usb/lp0",
            "--command", "not-a-manufacturer-command")
must_reject("--idle-confirmed", "--device", "/dev/usb/lp0",
            "--command", "status", "--output", "/tmp/unwanted.bin")
print("PASS probe CLI requires explicit idle flag, exact device path and known query; no --output files")
