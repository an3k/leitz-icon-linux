"""Regression of the new inventory bridge with synthetic inputs only."""
import importlib.util
import pathlib
import tempfile
import unittest

ROOT=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location("compare_windows_gpd_four_sources",ROOT / "compare_windows_gpd_four_sources.py")
# The existing inventory_windows_gpd.py is imported in the same directory in CI.
import sys
sys.path.insert(0,str(ROOT))
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class InventoryTests(unittest.TestCase):
    def setUp(self):
        self.linux={"Example":"Linux Original","Continuous70050001":'Continuous - 2 1/5"'}
        self.macos={"Example":"macOS Original","DHLShippingLabel":"DHLShippingLabel"}
        self.dev={"Example":{"label":"Dev Original","page":[10,20],"area":[8,19],"origin":[1,1]},
                  "DHLShippingLabel":{"label":"DHL Shipping Label","page":[1039,4204],"area":[960,4144],"origin":[40,30]}}
        self.options=[{"name":"Example","option_id":13,"resource_name_id":99,"page_dimensions":[10,20],
                       "printable_area":[8,18],"printable_origin":[1,1],"command_bytes_hex":"1b4c"},
                      {"name":"DHLShippingLabel","option_id":335,"resource_name_id":None,"page_dimensions":[1039,4204],
                       "printable_area":[960,4144],"printable_origin":[40,30],"command_bytes_hex":"1b4c08361b7703c0"},
                      {"name":"CUSTOMSIZE","option_id":None,"resource_name_id":None,"page_dimensions":None,
                       "printable_area":None,"printable_origin":None,"command_bytes_hex":"1b4c"}]
    def test_four_columns_and_bytes_without_fake_names(self):
        report=module.render(self.options,self.linux,self.macos,self.dev)
        self.assertIn("Linux 2; macOS 2; Windows 3; Eigenentwicklung 2",report)
        self.assertIn("| `DHLShippingLabel` | — | DHLShippingLabel | `DHLShippingLabel` | — | 335 | DHL Shipping Label | identische Geometriewerte |",report)
        self.assertIn("abweichend: printable_area",report)
        self.assertIn("variabel (nicht vergleichbar)",module.geometry_match(self.options[2], {"page":[4,5]}))
        self.assertIn("`1b4c08361b7703c0`",report)
        self.assertNotIn("Windows Originalname: DHL Shipping Label",report)
    def test_missing_distinct_and_duplicate(self):
        self.assertEqual(module.geometry_match(None,self.dev['Example']),"—")
        self.assertEqual(module.geometry_match(self.options[0],None),"—")
        with self.assertRaisesRegex(ValueError,"Duplicate Windows"):
            module.render(self.options + [self.options[0]],self.linux,self.macos,self.dev)
    def test_read_ppd_exact_labels(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=pathlib.Path(tmp)/"a.ppd"
            path.write_text('*PageSize Example/My Original: "thing"\n*PageSize Example2/Multi Word: "thing"\n')
            self.assertEqual(module.read_ppd(path),{"Example":"My Original","Example2":"Multi Word"})
            path.write_text('*PageSize A/One: "x"\n*PageSize A/Two: "x"\n')
            with self.assertRaisesRegex(ValueError,"Duplicate"):
                module.read_ppd(path)
    def test_hash_is_required_before_option_matching(self):
        with tempfile.TemporaryDirectory() as tmp:
            import zipfile
            path=pathlib.Path(tmp)/"original.zip"
            with zipfile.ZipFile(path,'w') as z:
                z.writestr(module.GPD_ZIP_PATH,'*Feature: PaperSize\n*Feature: Halftone\n')
            with self.assertRaisesRegex(ValueError,"Wrong Windows GPD SHA-256"):
                module.load_windows_original(path)

if __name__=='__main__':
    unittest.main()
