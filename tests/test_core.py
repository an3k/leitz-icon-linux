#!/usr/bin/env python3
"""Self-contained offline reference and corruption checks; no printer required."""
from pathlib import Path
from subprocess import run
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parent.parent
with TemporaryDirectory() as td:
    root = Path(td)
    pgm = root / "test.pgm"
    # Synthetic 960-pixel, 300x600 DPI raster; deliberately not the physical golden fixture.
    width, height = 960, 120
    with pgm.open("wb") as f:
        f.write(f"P5\n{width} {height}\n255\n".encode())
        for y in range(height):
            line = bytearray(b"\xff" * width)
            if y in (15, 119):
                line[10:30] = b"\x00" * 20
            f.write(line)
    dest = root / "test.bin"
    p = run([str(ROOT / "leitz-cli"), "pgm", str(pgm), str(dest)], capture_output=True)
    assert p.returncode == 0, p.stderr.decode()
    original = dest.read_bytes()
    assert run([str(ROOT / "leitz-cli"), "validate", str(dest)], capture_output=True).returncode == 0
    for drop in (1, 9, 150):
        bad = root / "bad.bin"
        bad.write_bytes(original[:-drop])
        assert run([str(ROOT / "leitz-cli"), "validate", str(bad)], capture_output=True).returncode != 0
    print("PASS generated 300x600 synthetic sample and corruption rejection")
