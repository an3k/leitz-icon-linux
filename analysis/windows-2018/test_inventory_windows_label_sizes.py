import unittest
from fractions import Fraction
from inventory_windows_label_sizes import collect_ui, inventory, parse_inch

XML=b'''<LabelSizeData><LabelSizes>
<LabelSize Id="Rotated"><Metre><Width>178</Width><Height>88</Height></Metre><Inch><Width>7</Width><Height>3 1/2</Height></Inch></LabelSize>
<LabelSize Id="Direct"><Metre><Width>88</Width><Height>51</Height></Metre><Inch><Width>3 1/2</Width><Height>2</Height></Inch></LabelSize>
<LabelSize Id="Continuous112"><Metre><Width>39</Width><Height/></Metre><Inch><Width>1 1/2</Width><Height/></Inch></LabelSize>
<LabelSize Id="MetricImperialRotated"><Metre><Width>76</Width><Height>50</Height></Metre><Inch><Width>2</Width><Height>3</Height></Inch></LabelSize>
</LabelSizes></LabelSizeData>'''
GPD=b'''*MasterUnits: PAIR(300,600)
*Feature: PaperSize
{
    *Option: Rotated
    {
      *OptionID: 501
      *PageDimensions: PAIR(1039,4204)
      *PrintableArea: PAIR(960,4144)
      *PrintableOrigin: PAIR(40,30)
      *Cmd: "<1B><4C><01><02>"
    }
    *Option: Direct
    {
      *OptionID: 502
      *PageDimensions: PAIR(1039,1200)
      *PrintableArea: PAIR(960,1140)
      *PrintableOrigin: PAIR(40,30)
      *Cmd: "<1B><4C><01><02>"
    }
    *Option: OnlyGPD
    {
      *OptionID: 503
      *PageDimensions: PAIR(300,600)
      *Cmd: "<1B><4C><01><02>"
    }
}
*Feature: Halftone
'''

class LabelSizeTests(unittest.TestCase):
    def test_inch_fields_and_rotated_exception(self):
        self.assertEqual(parse_inch("1 1/2"),Fraction(3,2))
        self.assertEqual(parse_inch("3/4"),Fraction(3,4))
        records=collect_ui(XML)
        self.assertEqual([r["metric_to_inch_orientation"] for r in records],
                         ["direct","direct","continuous_no_fixed_height","swapped"])

    def test_gpd_orientations_and_missing(self):
        result=inventory(XML,GPD)
        values={r["id"]:r for r in result["application_labels"]}
        self.assertEqual(values["Rotated"]["orientation"],"swapped")
        self.assertEqual(values["Direct"]["orientation"],"direct")
        self.assertEqual(values["Continuous112"]["orientation"],"not_in_gpd")
        self.assertEqual(values["Rotated"]["gpd_printable_area"],[960,4144])
        self.assertEqual(result["gpd_only_identifiers"],["OnlyGPD"])
        self.assertEqual(result["counts"]["shared_fixed_direct"],1)
        self.assertEqual(result["counts"]["shared_fixed_swapped"],1)

    def test_duplicate_rejected(self):
        with self.assertRaises(ValueError):
            collect_ui(XML.replace(b'Id="Direct"',b'Id="Rotated"'))

    def test_master_units_not_assumed(self):
        with self.assertRaises(ValueError):
            inventory(XML,GPD.replace(b"300,600",b"200,400"))

if __name__=="__main__":
    unittest.main()
