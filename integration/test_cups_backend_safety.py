#!/usr/bin/env python3
"""Offline CUPS backend safety contract; never opens a USB device."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
backend = ROOT / "cups/leitzusb"

for file in (backend,):
    checked = subprocess.run(["bash", "-n", str(file)], capture_output=True, text=True)
    assert checked.returncode == 0, (file, checked.stderr)

wire = backend.read_text(encoding="utf-8")

# A signal during transfer may leave a printed fragment. CUPS_BACKEND_FAILED
# (1) permits policy-driven replay; CANCEL (5) is final for this job.
cancel = re.search(r"(?ms)^on_cancel\(\)\s*\{(.*?)^\}", wire)
assert cancel, "Missing cancellation handler"
assert re.search(r"(?m)^\s*exit 5\s*$", cancel.group(1)), "Cancellation must not use retryable status"

# The USB backend only transports validated bytes; encoder selection
# belongs to the driver and its pluggable validators, not USB transport.
assert "set -euo pipefail" in wire
assert 'validate-auto "$tmp"' in wire
assert "MAX_WIDTH=" in wire
assert "PROTOCOL=Current" not in wire
assert "PROTOCOL=Native" not in wire
assert '$dialect' not in wire
assert "INFO: Leitz job transferred to USB" in wire
print("PASS backend cancellation, validated width, and protocol-independent transport")
