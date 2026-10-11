#!/usr/bin/env python3
"""Emit installed CUPS PPD from the single, unmodified catalog template.
A protocol not present as a source file is not an installable CUPS choice.
Never alter paper format entries or their vendor-provided names.
"""
from pathlib import Path
import sys

if len(sys.argv) != 3:
    raise SystemExit("Usage: build_ppd.py INPUT.ppd OUTPUT.ppd")
source = Path(sys.argv[1])
target = Path(sys.argv[2])
srcdir = Path("src")
present = {
    "Current": (srcdir / "leitz_protocol_current.c").is_file(),
    "NativeLinux103": (srcdir / "leitz_protocol_native_linux_103.c").is_file(),
}
available = [name for name, exists in present.items() if exists]
if not available:
    raise SystemExit("ERROR: No protocol source files available")
default = "Current" if present["Current"] else "NativeLinux103"
lines = []
for line in source.read_text(encoding="utf-8").splitlines(keepends=True):
    if line.startswith("*DefaultLeitzProtocol:"):
        line = f"*DefaultLeitzProtocol: {default}\n"
    if any(line.startswith(f"*LeitzProtocol {name}/") for name in present if not present[name]):
        continue
    lines.append(line)
target.parent.mkdir(parents=True, exist_ok=True)
target.write_text("".join(lines), encoding="utf-8")
print("CUPS print protocols: " + ", ".join(available))
