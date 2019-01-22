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
# This script is the 'preprocessor';
# it takes zone files from 'uncompleted'
# and puts them, if needed, in 'unsigned', with
# a couple of completions steps:
# - if there is a <zone>.ds file in input/keys for a direct
#   child of this, it's added to the zone
# - unless otherwise specified, the serial is updated to 'timestamp'
#   (with ldns-read-zone)

from workbench import env, zonedata, dnsutil, filestamps
import optparse
import os
import shutil
import subprocess

#DB_FILE = "static_zones.db"
INPUT_DIR = env.OUTPUT_BASE_PATH + "/uncompleted/"
OUTPUT_DIR = env.OUTPUT_BASE_PATH + "/unsigned/"
DS_DIR = env.OUTPUT_BASE_PATH + "/keys"

def get_all_zone_files():
    result = []
    for (dirpath, dirnames, filenames) in os.walk(INPUT_DIR):
        result.extend(filenames)
        break
    return result

def find_all_ds_names():
    # Conventions:
    # DS files are named <zone>.ds or <zone>.ksk.ds
    # If .ksk.ds is present, this one is used
    # otherwise, if .ds is present, that is used.
    # (that info is later used but I thought I'd mention it)
    # This 'just' returns a list of the znames
    # (see also the code of add_ds_file() later)
    result = []
    for (dirpath, dirnames, filenames) in os.walk(DS_DIR):
        for fn in filenames:
            dn = None
            if fn.endswith(".ksk.ds"):
                dn = fn[:-6]
            elif fn.endswith(".ds"):
                dn = fn[:-2]
            if dn is not None and dn not in result:
                result.append(dn)
    return result
    
def add_ds_file(result, ds_name):
    kskfile = DS_DIR + os.sep + ds_name + "ksk.ds"
    zskfile = DS_DIR + os.sep + ds_name + "ds"
    if os.path.exists(kskfile):
        result.append(kskfile)
    elif os.path.exists(zskfile):
        result.append(zskfile)

def add_ds_to_zone(ds_file, z_file):
    with open(ds_file, 'r') as ds:
        with open(z_file, 'a') as z:
            for line in ds.readlines():
                z.write(line)

def update_serial_and_copy(zname, z_infile, z_outfile):
    cmd = [ "ldns-read-zone",
            "-s", "-S", "unixtime"
          ]
# MD let's do this instead
#    cmd = [ "ldns-read-zone",
#            "-s", "-S", "YYYYMMDDxx"
#          ]          
    with open(z_infile, 'rb') as inf:
        with open(z_outfile, 'wb') as out:
            p = subprocess.Popen(cmd, stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE)
            originline = "$ORIGIN %s\n" % zname
            p.stdin.write(bytes(originline, 'ASCII'))
            for line in inf.readlines():
                p.stdin.write(line)
            (stdout, stderr) = p.communicate();
            out.write(stdout)
            # TODO: check exitcode?

def complete_zones(zones, regen_all):
    ds_names = find_all_ds_names()
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    for z in zones:
        zname = dnsutil.fqdn(z)
        z_infile = INPUT_DIR + z
        z_outfile = OUTPUT_DIR + "/" + z

        # First determine which files need update-checks
        ds_files = []
        for ds_name in ds_names:
            if dnsutil.is_direct_parent(zname, ds_name):
                add_ds_file(ds_files, ds_name)

        if filestamps.file_updated([z_infile] + ds_files, z_outfile) or\
           regen_all:
            # Update the serial. This also copies the zone file to the
            # next step
            update_serial_and_copy(zname, z_infile, z_outfile)
            
            for ds_file in ds_files:
                add_ds_to_zone(ds_file, z_outfile)
            
if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Rewrite ALL the zone files")
    (options, args) = parser.parse_args()
    complete_zones(get_all_zone_files(), options.regen)
