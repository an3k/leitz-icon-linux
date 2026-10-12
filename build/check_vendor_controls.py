#!/usr/bin/env python3
"""Offline source-backed media/catalog and read-only command checks."""
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
catalog = json.loads((ROOT / "media/vendor_macos_catalog.json").read_text())
backend = (ROOT / "cups/leitzusb").read_text()
assert len(catalog["cartridges"]) == 23
assert len(catalog["paper_formats"]) == 22
skus = {c["sku"] for c in catalog["cartridges"]}
assert len(skus) == 23
for record in catalog["paper_formats"]:
    assert record["compatible_skus"], record["page_size"]
    expected = f"  {record['page_size']}) allowed_skus=' " + " ".join(record["compatible_skus"]) + " ' ;;"
    assert expected in backend, record["page_size"]
    assert set(record["compatible_skus"]) <= skus, record["page_size"]
assert "IsCurrentInsertedCartridgeDieCut=YES" in backend
assert "PageSize=([[:alnum:]_]+)" in backend
assert "SKU:([0-9]{8,9})" in backend

filter_script = ROOT / "cups/leitz-command"
cases = {
    "ReportStatus": "REPORT_STATUS",
    "GetPrinterStatus": "REPORT_STATUS",
    "GetFirmwareVersion": "GET_FIRMWARE_VERSION",
    "GetWiFiState": "GET_WIFI_STATE",
}
for command, frame in cases.items():
    proc = subprocess.run(
        ["bash", str(filter_script), "1", "a", "name", "1", ""],
        input=f"#CUPS-COMMAND\n{command}\n",
        text=True, capture_output=True, check=True,
    )
    assert proc.stdout == f"LEITZ-CMD-V1:{frame}\n", (command, proc.stdout)
for contents in (
    "#CUPS-COMMAND\nBootMode\n",
    "#CUPS-COMMAND\nGetFirmwareVersion\nGetWiFiState\n",
    "#CUPS-COMMAND\nGetFirmwareVersion=xxx\n",
):
    proc = subprocess.run(
        ["bash", str(filter_script), "1", "a", "name", "1", ""],
        input=contents, text=True, capture_output=True,
    )
    assert proc.returncode != 0, contents
print("Vendor media cross-reference and CUPS read-only command checks: PASS")
