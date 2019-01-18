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

import os

INPUT_EXTENSION = ".page"
OUTPUT_EXTENSION = ".html"

files = []
for (dirpath, dirnames, filenames) in os.walk("site_parts"):
    files.extend([filename[:-len(INPUT_EXTENSION)] for filename in filenames if filename.endswith(".page")])

for file in files:
    print(file)
    cmd = "cat site_parts/header.inc site_parts/%s.page site_parts/footer.inc > site/%s.html" % (file, file)
    os.system(cmd)
