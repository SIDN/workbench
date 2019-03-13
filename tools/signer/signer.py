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


# For each zone entry that has a non-empty 'signer' field
# Call the signer
# with params and keys
#

# Check whether necessary by checking unsigned/zname and keys in fs
from workbench import dnsutil, env, filestamps, zonedata
import optparse
import os
import shutil
import subprocess
import sys

INPUT_DIR = env.OUTPUT_BASE_PATH + "/unsigned"
# signer should have output dir as well (shit), but this is for copy
OUTPUT_DIR = env.OUTPUT_BASE_PATH + "/signed"
KEYS_DIR = env.OUTPUT_BASE_PATH + "/keys"

# this needs to be in util too
class CWD:
    def __init__(self, new_wd):
        self.new_wd = new_wd
    
    def __enter__(self):
        self.orig_wd = os.getcwd()
        os.chdir(self.new_wd)
    
    def __exit__(self, type, value, traceback):
        os.chdir(self.orig_wd)

def copy_or_sign_zone(zd):
    # only do this if there is a master
    if zd.get("nofile"):
        return
    signer = zd.get("signer_script")
    zname = dnsutil.ufqdn(zd.get("name"))
    infile = INPUT_DIR + "/" + zname
    outfile = OUTPUT_DIR + "/" + zname
    if signer == "":
        # just copy
        shutil.copy(infile, outfile)
    else:
        cmd = [ signer ]
        cmd.extend(zd.get("signer_params"))
        cmd.extend([KEYS_DIR + "/" + k for k in zd.get("signer_keys")])
        with CWD(env.OUTPUT_BASE_PATH):   
            try:
                p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            except Exception:
                print("Error calling external program: " + " ".join(cmd))
                raise
            out, err = p.communicate()
            # TODO: check rcode
            if p.returncode != 0:
                sys.stderr.write(str(err))
                raise Exception("Error calling subprocess: " + " ".join(cmd))
        

def copy_or_sign_zones(regen_all):
    all_zones = dnsutil.read_all_db_files()
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    # needed info: zone file name, zone origin, signer commands
    #print([str(zd) for zd in all_zones])
    for z in all_zones:
        needed_files = [ INPUT_DIR + "/" + dnsutil.ufqdn(z.get("name")) ]
        output_file = OUTPUT_DIR + "/" + dnsutil.ufqdn(z.get("name"))
        if len(z.get("signer_keys")) > 0:
            needed_files.extend([KEYS_DIR + "/" + k for k in z.get("signer_keys")])
        if filestamps.file_updated(needed_files, output_file):
            print("[signer] Creating zone " + z.get("name"))
            copy_or_sign_zone(z)

if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Re-sign/copy ALL the zone files")
    (options, args) = parser.parse_args()
    copy_or_sign_zones(options.regen)
