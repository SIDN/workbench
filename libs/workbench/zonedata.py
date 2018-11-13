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


#
# DNS Workbench
#
# This class represents a zone data object, specifying
# its name, where its contents comes from, and how it should
# be handled to generate the final zone file, as well as which
# servers should run it and how
#
import collections
import copy
import csv
import ast
import os

def str_quoter(value, separator = ',', quote = '"', escape = '\\'):
    string = str(value)
    if string.find(separator) > -1:
        string = string.replace(escape + quote, quote)
        string = string.replace(quote, escape + quote)
        return "%s%s%s" % (quote, string, quote)
    else:
        return string

class ZoneDataEmpty(Exception):
    pass
    
class ZoneDataVersion(Exception):
    pass

class ZoneDataProperty(Exception):
    pass

class ZoneData:
    PROPERTIES=collections.OrderedDict([
        ("version", 1),
        ("name", ""), # Should error later if this remains None
        ("signer_script", ""),
        ("signer_params", []),
        ("signer_keys", []),
        ("finalizer_script", ""),
        ("finalizer_params", []),
        ("primary_names", []), # None == all, empty list = none
        ("secondary_names", []), # none has no meaning!
        ("nofile", False) # no associated file
    ])

    def __init__(self):
        self.properties = copy.deepcopy(ZoneData.PROPERTIES)

    def check(self):
        """
        Basic check to see if the object has been filled
        Raises ZoneDataEmpty if not
        Raises ZoneDataVersion if version does not match
        """
        if self.properties["name"] == "":
            raise ZoneDataEmpty()
        zdv = self.properties["version"]
        cdv = ZoneData.PROPERTIES["version"]
        if zdv != cdv:
            raise ZoneDataVersion("Wrong version: %s, should be %s" %
                                  (str(zdv), str(cdv)))

    def set(self, item, value):
        if item not in self.properties:
            raise ZoneDataProperty("Unknown property: %s" % item)
        if self.properties[item] is not None and value is not None and\
           type(value) != type(self.properties[item]):
            raise ZoneDataProperty("Bad type for property: %s" % item)
        self.properties[item] = value

    def get(self, item):
        if item not in self.properties:
            raise ZoneDataProperty("Unknown property: %s" % item)
        return self.properties[item]

    def add(self, item, value):
        # only works on list items
        if item not in self.properties:
            raise ZoneDataProperty("Unknown property: %s" % item)
        if type(self.properties[item]) is not list:
            raise ZoneDataProperty("Bad type for property %s: %s != %s" % (item, str(type(self.properties[item])), str(list)))
        self.properties[item].append(value)

    def __str__(self):
        """
        Returns the values in a comma-separated string
        """
        self.check()
        line = ", ".join([str_quoter(e) for e in self.properties.values()])
        return line
    
    def get_value_list(self):
        return [str(v) for v in self.properties.values()]
    
    def set_value_list(self, values):
        for h in self.get_headers():
            s = values.pop(0)
            try:
                v = ast.literal_eval(s)
            except:
                v = s
            self.properties[h] = v

    @classmethod
    def get_headers(self):
        return ZoneData.PROPERTIES.keys()

def read_zone_data(filename):
    with open(filename, 'r') as r:
        reader = csv.reader(r)
        result = []
        # drop the header line
        r.readline()
        for r in reader:
            # This is a good place to do automatic version updatin',
            # btw
            # Perhaps by header name matching?
            zd = ZoneData()
            zd.set_value_list(r)
            zd.check()
            result.append(zd)
        
        return result

def write_zone_data(filename, zone_data_list):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, 'w') as f:
        writer = csv.writer(f)
        writer.writerow(list(ZoneData.get_headers()))
        writer.writerows([zd.get_value_list() for zd in zone_data_list])

# some simple test code
if __name__ == "__main__":
    empty_data = ZoneData()
    print(empty_data.to_csv())
