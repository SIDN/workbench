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
INPUT_DIR = "site_parts"
OUTPUT_DIR = "site"
ASSETS_DIR = "%s/assets" % INPUT_DIR

if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

files = []
for (dirpath, dirnames, filenames) in os.walk(INPUT_DIR):
    files.extend([filename[:-len(INPUT_EXTENSION)] for filename in filenames if filename.endswith(".page")])

os.system("cp -r %s %s" % (ASSETS_DIR, OUTPUT_DIR))

for sfile in files:
    print(sfile)
    cmd = "cat %(i)s/header.inc %(i)s/%(f)s.page %(i)s/footer.inc > %(o)s/%(f)s.html" %\
          {
            'f': sfile,
            'i': INPUT_DIR,
            'o': OUTPUT_DIR
          }

    #(file, INPUT_DIR, file)
    os.system(cmd)

