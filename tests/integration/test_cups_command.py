#!/usr/bin/env python3
"""CUPS command filter offline tests: never opens USB or prints a label."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FILTER = ROOT / "driver" / "cups" / "leitz-command"

def run(data):
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "command.txt"
        p.write_bytes(data)
        return subprocess.run([str(FILTER), "1", "root", "Maintenance", "1", "", str(p)],
                              capture_output=True, timeout=5)

for command in (b"ReportStatus", b"reportstatus", b"REPORTSTATUS"):
    p = run(b"#CUPS-COMMAND\n# comment\n" + command + b"\n")
    assert p.returncode == 0, p.stderr
    assert p.stdout == b"LEITZ-CMD-V1:REPORT_STATUS\n"
for bad in (b"", b"ReportStatus\n", b"#CUPS-COMMAND\n",
            b"#CUPS-COMMAND\nClean all\n",
            b"#CUPS-COMMAND\nPrintSelfTestPage\n",
            b"#CUPS-COMMAND\nReportStatus now\n",
            b"#CUPS-COMMAND\nReportStatus\n\x00Clean all\n",
            b"#CUPS-COMMAND\n" + b"X" * 8193):
    p = run(bad)
    assert p.returncode != 0 and not p.stdout, (bad[:30],p.stderr)
p=run(b"#CUPS-COMMAND\r\nReportStatus\r\nReportStatus\n")
assert p.returncode==0 and p.stdout==b"LEITZ-CMD-V1:REPORT_STATUS\n"
print("PASS CUPS ReportStatus control command, rejection cases and bounded input")
