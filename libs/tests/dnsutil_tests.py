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


from workbench import dnsutil
import unittest

class DNSUtilTests(unittest.TestCase):
    def test_fqdn(self):
        self.assertEqual(".", dnsutil.fqdn(""))
        self.assertEqual(".", dnsutil.fqdn("."))
        self.assertEqual("foo.", dnsutil.fqdn("foo"))
        self.assertEqual("foo.", dnsutil.fqdn("foo."))
        self.assertEqual("foo..", dnsutil.fqdn("foo.."))
        self.assertEqual("foo.bar.", dnsutil.fqdn("foo.bar"))
        self.assertEqual("foo.bar.", dnsutil.fqdn("foo.bar."))

    def test_is_direct_parent(self):
        self.assertTrue(dnsutil.is_direct_parent("b.c", "a.b.c"))
        self.assertTrue(dnsutil.is_direct_parent("b.c", "a.b.c."))
        self.assertTrue(dnsutil.is_direct_parent("b.c.", "a.b.c"))
        self.assertTrue(dnsutil.is_direct_parent("b.c.", "a.b.c."))
        self.assertTrue(dnsutil.is_direct_parent("c", "b.c"))
        self.assertTrue(dnsutil.is_direct_parent("c", "b.c."))
        self.assertTrue(dnsutil.is_direct_parent("c.", "b.c"))
        self.assertTrue(dnsutil.is_direct_parent("c.", "b.c."))
        self.assertTrue(dnsutil.is_direct_parent("", "c"))
        self.assertTrue(dnsutil.is_direct_parent("", "c."))
        self.assertTrue(dnsutil.is_direct_parent(".", "c"))
        self.assertTrue(dnsutil.is_direct_parent(".", "c."))

        self.assertFalse(dnsutil.is_direct_parent("b.c", "c"))
        self.assertFalse(dnsutil.is_direct_parent("b.c", "c."))
        self.assertFalse(dnsutil.is_direct_parent("b.c.", "c"))
        self.assertFalse(dnsutil.is_direct_parent("b.c.", "c."))
        self.assertFalse(dnsutil.is_direct_parent("b.c", "c"))
        self.assertFalse(dnsutil.is_direct_parent("b.c", "c."))
        self.assertFalse(dnsutil.is_direct_parent("b.c.", "c"))
        self.assertFalse(dnsutil.is_direct_parent("b.c.", "c."))
        self.assertFalse(dnsutil.is_direct_parent("c", ""))
        self.assertFalse(dnsutil.is_direct_parent("c", "."))
        self.assertFalse(dnsutil.is_direct_parent("c.", ""))
        self.assertFalse(dnsutil.is_direct_parent("c.", "."))

        self.assertFalse(dnsutil.is_direct_parent("", ""))
        self.assertFalse(dnsutil.is_direct_parent("", "."))
        self.assertFalse(dnsutil.is_direct_parent(".", ""))
        self.assertFalse(dnsutil.is_direct_parent(".", "."))

if __name__ == '__main__':
    unittest.main()
