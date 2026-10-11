#!/usr/bin/env python3
"""Compile/test each codec independently after physically removing the other.
No USB; no changes to the checked-out working tree.
"""
import shutil
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
variants = [("Current", "leitz_protocol_native_linux_103.c", "NativeLinux103"),
            ("NativeLinux103", "leitz_protocol_current.c", "Current")]
for wanted, omitted, forbidden in variants:
    with tempfile.TemporaryDirectory(prefix="leitz-isolation-") as t:
        base = Path(t)
        for folder in ("src", "cups", "tests"):
            shutil.copytree(root / folder, base / folder)
        (base / "tools").mkdir()
        shutil.copy2(root / "tools/build_ppd.py", base / "tools/build_ppd.py")
        shutil.copy2(root / "Makefile", base / "Makefile")
        (base / "src" / omitted).unlink()
        # No code generation or link fallback: absent module means no symbol.
        run = subprocess.run(["make", "all", "mock-filter", "ppd"],
                             cwd=base, capture_output=True, text=True, timeout=90)
        assert run.returncode == 0, (wanted, run.stdout, run.stderr)
        ppd = (base / "dist/LeitzIcon.ppd").read_text()
        assert f"*DefaultLeitzProtocol: {wanted}" in ppd
        assert f"*LeitzProtocol {wanted}/" in ppd
        assert f"*LeitzProtocol {forbidden}/" not in ppd
        code = subprocess.run([str(base / "leitz-cli"),
                               "validate-native" if forbidden == "NativeLinux103" else "validate",
                               "/dev/null"], capture_output=True, text=True)
        assert code.returncode == 2 and "not installed" in code.stderr, code.stderr
        print("PASS isolated", wanted, "without", forbidden, "and filtered PPD")
