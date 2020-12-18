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


# For each zone entry that has a non-empty 'finalizer_script' field
# Call the finalizer script with params
# If the field is empty, copy the file from signed/

# Check whether necessary by checking unsigned/zname and keys in fs
from workbench import dnsutil, env, filestamps, zonedata
import optparse
import os
import shutil
import subprocess
import sys

INPUT_DIR = env.OUTPUT_BASE_PATH + "/signed"
# signer should have output dir as well (shit), but this is for copy
OUTPUT_DIR = env.OUTPUT_BASE_PATH + "/final"

# this needs to be in util too
class CWD:
    def __init__(self, new_wd):
        self.new_wd = new_wd
    
    def __enter__(self):
        self.orig_wd = os.getcwd()
        os.chdir(self.new_wd)
    
    def __exit__(self, type, value, traceback):
        os.chdir(self.orig_wd)

def finalize_or_sign_zone(zd):
    # only do this if there is a master
    if zd.get("nofile"):
        return

    finalizer = zd.get("finalizer_script")
    zname = dnsutil.ufqdn(zd.get("name"))
    infile = INPUT_DIR + "/" + zname
    outfile = OUTPUT_DIR + "/" + zname
    if finalizer == "":
        # just copy
        shutil.copy(infile, outfile)
    else:
        cmd = [ finalizer ]
        cmd.extend(zd.get("finalizer_params"))
        with CWD(env.OUTPUT_BASE_PATH):   
            try:
                p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            except Exception:
                print("Error calling external program: " + " ".join(cmd))
                raise
            out, err = p.communicate()
            # TODO: check rcode
            if p.returncode != 0:
                sys.stderr.write("Error from subprocess stderr:\n")
                sys.stderr.write(err.decode("utf-8"))
                raise Exception("Error calling subprocess: " + " ".join(cmd))
        

def finalize_or_sign_zones(regen_all):
    all_zones = dnsutil.read_all_db_files()
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    for z in all_zones:
        needed_files = [ INPUT_DIR + "/" + dnsutil.ufqdn(z.get("name")) ]
        output_file = OUTPUT_DIR + "/" + dnsutil.ufqdn(z.get("name"))
        if filestamps.file_updated(needed_files, output_file):
            print("[finalizer] Creating zone " + z.get("name"))
            finalize_or_sign_zone(z)

if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Re-sign/copy ALL the zone files")
    (options, args) = parser.parse_args()
    finalize_or_sign_zones(options.regen)
