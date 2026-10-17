#!/usr/bin/env python3
"""Regression: Windows GPD original text transcription versus 77-record inventory."""
import json
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
from inventory_windows_gpd import paper_options
from compare_windows_gpd_four_sources import read_ppd, read_development, render

GPD = HERE / "LeitzIcon_2018_GPD_text_transcription.gpd.txt"
INVENTORY = HERE / "windows_2018_original_media_inventory.json"
REPORT = HERE / "FOUR_SOURCE_GPD_COMPARISON_COMPLETE.md"
LINUX = ROOT / "originals/linux-1.0.3/LeitzIcon.ppd"
MAC = ROOT / "originals/macos-5.1.465-classic/LeitzIcon.ppd"
GPD_SHA = "f81130f358b6dbc6099812d8ab243958854436be4b1e8c0c0ef8ec7fb60954b7"

class WindowsGpdSourceTests(unittest.TestCase):
    def setUp(self):
        self.saved = json.loads(INVENTORY.read_text(encoding="utf-8"))
        self.parsed = paper_options(GPD.read_text(encoding="utf-8"))

    def test_all_original_fields_from_77_options(self):
        self.assertEqual(self.saved["gpd_sha256"], GPD_SHA)
        self.assertEqual(self.saved["original_win_fixed_count"], 76)
        self.assertEqual(self.saved["original_win_custom_count"], 1)
        self.assertEqual(self.saved["original_win_total_count"], 77)
        self.assertEqual(self.parsed, self.saved["options"])
        self.assertEqual(len(self.parsed), 77)
        self.assertEqual(len({x["name"] for x in self.parsed}), 77)
        fixed = [x for x in self.parsed if x["page_dimensions"] is not None]
        self.assertEqual(len(fixed), 76)
        self.assertEqual(len({x["option_id"] for x in fixed}), 76)
        self.assertNotIn(None, {x["option_id"] for x in fixed})
        self.assertEqual([x["name"] for x in self.parsed
                          if x["page_dimensions"] is None], ["CUSTOMSIZE"])
        custom = self.parsed[-1]
        self.assertEqual(custom["min_size"], [16,16])
        self.assertEqual(custom["max_size"], [10000,64000])

    def test_exact_development_geometry_for_all_76_fixed(self):
        dev = read_development(sys.argv[1])
        fixed = [o for o in self.parsed if o["page_dimensions"] is not None]
        self.assertEqual(len(dev), 86)
        for o in fixed:
            with self.subTest(name=o["name"]):
                self.assertIn(o["name"], dev)
                item = dev[o["name"]]
                self.assertEqual(o["page_dimensions"], item["page"])
                self.assertEqual(o["printable_area"], item["area"])
                self.assertEqual(o["printable_origin"], item["origin"])
        self.assertEqual(set(dev) - {o["name"] for o in fixed},
            {"Continuous112","LargePendaflexTabRotated","LeverArchFile1005",
             "Multipurpose70030001Rotated","NameBadgeCard70190001Rotated",
             "ShippingLabel70180001Rotated","LeitzContinuous112",
             "LeitzDHLShippingLabel","LeitzNameBadgeCard8214",
             "LeitzWristband"})

    def test_complete_four_source_report_reproducible(self):
        report = render(self.parsed, read_ppd(LINUX), read_ppd(MAC),
                        read_development(sys.argv[1]))
        self.assertEqual(REPORT.read_text(encoding="utf-8"), report)
        self.assertIn("Linux 40; macOS 41; Windows 77;",report)
        self.assertIn("Eigenentwicklung 86; vereinigte exakte IDs 87",report)
        self.assertIn("1b4c08361b7703c0", report)

    def test_original_length_exceptions_are_retained(self):
        continuous = []
        exceptions = []
        for o in self.parsed:
            if o["page_dimensions"] is None: continue
            raw = bytes.fromhex(o["command_bytes_hex"])
            self.assertEqual(len(raw), 8)
            self.assertEqual(raw[:2], b"\x1bL")
            self.assertEqual(raw[4:6], b"\x1bw")
            width = int.from_bytes(raw[6:8], "big")
            length = int.from_bytes(raw[2:4], "big")
            self.assertEqual(width, o["printable_area"][0])
            if o["page_dimensions"][1] == 64000:
                continuous.append(o["name"])
                self.assertEqual(length, 65535)
            elif length * 2 != o["page_dimensions"][1]:
                exceptions.append(o["name"])
                self.assertEqual(length,570)
                self.assertEqual(o["page_dimensions"][1],1200)
        self.assertEqual(len(continuous),20)
        self.assertEqual(set(exceptions),{"AppointmentCard","LeitzAppointmentCard"})

if __name__=="__main__":
    if len(sys.argv)!=2:
        sys.exit("Usage: test_windows_gpd_full_inventory.py PINNED_DRIVER_MEDIA_JSON")
    unittest.main(argv=[sys.argv[0]],verbosity=2)
