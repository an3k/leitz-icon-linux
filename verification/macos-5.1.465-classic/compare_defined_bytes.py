#!/usr/bin/env python3
"""Reference-only matcher: never asserts equality of undefined OEM heap bytes.

The classic manufacturer modes 0/1 send stride+4 bytes per raster row.
The final four bytes of each printed page are outside its raster allocation
and were observed to contain unrelated process memory, rather than CUPS
pixels. All other bytes must match exactly. Do not apply this exception to
other optimization modes.
"""
from pathlib import Path
import sys

a = Path(sys.argv[1]).read_bytes()
b = Path(sys.argv[2]).read_bytes()
# OEM _EndPage transmits ESC G only with CutMedia/LabelCutter OFF;
# ESC G ESC E is its four-byte cutter-ON boundary. Optional argument
# selects the real original command, not a heuristic offset skip.
footer = bytes([0x1B, 0x47]) if sys.argv[3:] == ["--cutter-off"] else bytes([0x1B, 0x47, 0x1B, 0x45])
def footers(data):
    return [i for i in range(len(data) - 3) if data[i:i+4] == footer]
if len(a) != len(b):
    print("DEFINED_BYTES=NO LENGTH_MISMATCH", len(a), len(b))
    sys.exit(1)
pa, pb = footers(a), footers(b)
if pa != pb or len(pa) != 2:
    print("DEFINED_BYTES=NO PAGE_BOUNDARY_MISMATCH", pa, pb)
    sys.exit(1)
undefined = set()
for offset in pa:
    undefined.update(range(offset - 4, offset))
other = [(i, a[i], b[i]) for i in range(len(a))
         if i not in undefined and a[i] != b[i]]
print(f"DEFINED_BYTES={'YES' if not other else 'NO'}"
      f" DOCUMENTED_UNDEFINED_BYTES={len(undefined)}"
      f" FIRST_OTHER_MISMATCHES={other[:8]}")
sys.exit(0 if not other else 1)
