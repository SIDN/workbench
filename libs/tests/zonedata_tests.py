#!/usr/bin/python3

#
# Copyright 2017 SIDN
# Written by Jelte Jansen
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#


from workbench import zonedata
from testutils import FileDeleter
import unittest

class ZoneDataTests(unittest.TestCase):
    def test_empty_zonedata(self):
        zd = zonedata.ZoneData()
        self.assertRaises(zonedata.ZoneDataEmpty, zd.check)

    def test_bad_version(self):
        zd = zonedata.ZoneData()
        zd.set("name", "foo.")
        zd.set("version", 0)
        self.assertRaises(zonedata.ZoneDataVersion, zd.check)
        
    def test_bad_properties(self):
        zd = zonedata.ZoneData()
        zd.set("name", "foo.")
        self.assertRaises(zonedata.ZoneDataProperty, zd.set, "name", 1)
        self.assertRaises(zonedata.ZoneDataProperty, zd.set, "doestnotexist", 1)
        self.assertRaises(zonedata.ZoneDataProperty, zd.get, "doestnotexist")
    
    def test_headers(self):
        line = ", ".join(zonedata.ZoneData.get_headers())
        self.assertEqual(line,
                         "version, name, " +
                         "signer_script, signer_params, signer_keys, " +
                         "finalizer_script, finalizer_params, " +
                         "primary_names, secondary_names")

    def test_str(self):
        zd = zonedata.ZoneData()
        zd.set("name", "foo.")
        self.assertEqual(str(zd),
                         "1, foo., , [], [], , [], [], []")
        zd.set("name", "com,ma")
        self.assertEqual(str(zd),
                         '1, "com,ma", , [], [], , [], [], []')
        zd.set("name", "com\"ma")
        self.assertEqual(str(zd),
                         '1, com"ma, , [], [], , [], [], []')
        zd.set("name", "com\"m,a")
        self.assertEqual(str(zd),
                         '1, "com\\"m,a", , [], [], , [], [], []')

    def test_read_write_csv(self):
        zds = []
        for i in range(5):
            zd = zonedata.ZoneData()
            zd.set("name", "foo.%d.bar." % i)
            zds.append(zd)
        
        with FileDeleter(delete=True) as fd:
            zonedata.write_zone_data(fd.get_filename(), zds)
            
            zds2 = zonedata.read_zone_data(fd.get_filename())

if __name__ == '__main__':
    unittest.main()
