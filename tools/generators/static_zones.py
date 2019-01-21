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
# This generator takes a number of static zones; i.e. zones
# that we edit manually.
# For each zone file that has changed, in effect it is simply copied.
#
# However, a few tools are used; e.g. ldns-3597
#

from workbench import dnsutil, env, filestamps, zonedata
import optparse
import glob
import os
import shutil

print(env.ZONE_DB_PATH)

OUTPUT_FILE = "static_zones.db"
INPUT_DIR = env.INPUT_BASE_PATH + "/static_zones"

# TODO: is this still the intention? Why was it done in the first place?
#T3597 = "A6 CDS GPOS NINFO NSAP-PTR TLSA TALINK NID L32 L64 LP RKEY"
T3597 = "A6 CDS GPOS NSAP-PTR TLSA TALINK NID L32 L64 LP"
SIGNER = env.EXT_TOOLS_PATH + "/ldns-sign-special/ldns-sign-special"

def list_files(recursive=False, filter=None):
    result = []
    for (dirpath, dirnames, filenames) in os.walk(INPUT_DIR + "/auto"):
        # TODO: filter
        result.extend([ (fn, dirpath + "/" + fn) for fn in filenames ])
        if not recursive:
            break
    print(result)
    return result

def add_nsec3_sign_options(zd):
    dname = zd.get("name")
    dname_u = dnsutil.ufqdn(dname)
    zd.set("signer_script", SIGNER)
    zd.add("signer_params", "-o")
    zd.add("signer_params", dname)
    zd.add("signer_params", "-e")
    zd.add("signer_params", "20200101000000")
    zd.add("signer_params", "-f")
    zd.add("signer_params", "signed/" + dname_u)
    # Make this stdin too?
    zd.add("signer_params", "unsigned/" + dname_u)
    zd.add("signer_params", "-n")
    zd.add("signer_params", "-t")
    zd.add("signer_params", "5")
    zd.add("signer_params", "-s")
    zd.add("signer_params", "beef")
    zd.add("signer_keys", dname_u)
    zd.add("signer_keys", dname_u + ".ksk")

def add_nsec3_opt_out_sign_options(zd):
    zd.add("signer_params", "-p")
    return add_nsec3_sign_options(zd)

def add_3597(zd):
    zd.set("finalizer_script", "../ext/ldns-3597/ldns-3597")
    zd.add("finalizer_params", "-i")
    zd.add("finalizer_params", "signed/" + dnsutil.ufqdn(zd.get("name")))
    zd.add("finalizer_params", "-o")
    zd.add("finalizer_params", "final/" + dnsutil.ufqdn(zd.get("name")))
    for t in T3597.split(" "):
        zd.add("finalizer_params", t)

def set_servers(zd):
    # Most servers don't like to read/parse/load a number of rrtypes,
    # but they will accept it when they are secondary
    # So we'll make BIND9 (was NSD) the master, and all the others the secondary
    # For the types zones
    #zd.add("primary_names", "nsd")
    zd.add("primary_names", "bind9")    
    zd.add("secondary_names", "nsd4")
    zd.add("secondary_names", "knot")
    zd.add("secondary_names", "yadifa")

def generate_static_zone_entries():
    zds = []

    zd = zonedata.ZoneData()
    zd.set("name", "types." + env.DOMAIN)
    add_3597(zd)
    set_servers(zd)
    zds.append(zd)

    # this one is based on the previous one so it isn't done through
    # auto/
    zd = zonedata.ZoneData()
    dname = "types-signed." + env.DOMAIN
    zd.set("name", dname)
    dnsutil.add_standard_sign_options(zd)
    add_3597(zd)
    set_servers(zd)
    zds.append(zd)

    # Apex cname works with powerdns only
    zd = zonedata.ZoneData()
    dname = "apexcname." + env.DOMAIN
    zd.set("name", dname)
    zd.add("primary_names", "powerdns")
    dnsutil.add_standard_sign_options(zd)
    zds.append(zd)

    zd = zonedata.ZoneData()
    dname = "wildcards-nsec3." + env.DOMAIN
    zd.set("name", dname)
    add_nsec3_sign_options(zd)
    set_servers(zd)
    zds.append(zd)

    zd = zonedata.ZoneData()
    dname = "nsec3-opt-out." + env.DOMAIN
    zd.set("name", dname)
    add_nsec3_opt_out_sign_options(zd)
    zd.set("finalizer_script", "../ext/concat.sh")
    zd.add("finalizer_params", "final/" + dnsutil.ufqdn(zd.get("name")))
    zd.add("finalizer_params", "signed/" + dnsutil.ufqdn(zd.get("name")))
    zd.add("finalizer_params", "../input/static_zones/" + dnsutil.ufqdn(zd.get("name")) + ".opt-out")
    set_servers(zd)
    zds.append(zd)
 
    # Create an entry for each file in auto
    for af in list_files():
        autofiles = list_files()
        zd = zonedata.ZoneData()
        zd.set("name", af[0])
        dnsutil.add_standard_sign_options(zd)
        zds.append(zd)

    # TODO: remove ?
    ## Secondary for example.nl on nsd.sidnlabs.nl
    #zd = zonedata.ZoneData()
    #zd.set("name", "example.nl")
    ## explicitely set primary_names to none, not empty list
    #zd.set("nofile", True)
    #zd.add("primary_names", "94.198.159.3")
    #zd.add("secondary_names", "nsd")
    #zds.append(zd)

    zonedata.write_zone_data(env.ZONE_DB_PATH + "/static_zones.db", zds)
    #print([str(zd) for zd in zds])
    
    
def create_zone_files(regen = False):
    os.makedirs(env.OUTPUT_BASE_PATH + "/uncompleted", exist_ok=True)
    # in essence, simply copy them to uncompleted
    # unless regen is specified, only copy if original is updated
    source_file = INPUT_DIR + "/types.wb.sidnlabs.nl"
    target_file = env.OUTPUT_BASE_PATH + "/uncompleted/types.wb.sidnlabs.nl"
    if regen or filestamps.file_updated([source_file], target_file):
        shutil.copyfile(source_file, target_file)
    target_file = env.OUTPUT_BASE_PATH + "/uncompleted/types-signed.wb.sidnlabs.nl"
    if regen or filestamps.file_updated([source_file], target_file):
        shutil.copyfile(source_file, target_file)

    source_file = INPUT_DIR + "/apexcname.wb.sidnlabs.nl"
    target_file = env.OUTPUT_BASE_PATH + "/uncompleted/apexcname.wb.sidnlabs.nl"
    if regen or filestamps.file_updated([source_file], target_file):
        shutil.copyfile(source_file, target_file)

    source_file = INPUT_DIR + "/wildcards-nsec3.wb.sidnlabs.nl"
    target_file = env.OUTPUT_BASE_PATH + "/uncompleted/wildcards-nsec3.wb.sidnlabs.nl"
    if regen or filestamps.file_updated([source_file], target_file):
        shutil.copyfile(source_file, target_file)

    source_file = INPUT_DIR + "/nsec3-opt-out.wb.sidnlabs.nl"
    target_file = env.OUTPUT_BASE_PATH + "/uncompleted/nsec3-opt-out.wb.sidnlabs.nl"
    if regen or filestamps.file_updated([source_file], target_file):
        shutil.copyfile(source_file, target_file)

    # Copy all files in auto
    for (target_name, source) in list_files():
        target = env.OUTPUT_BASE_PATH + "/uncompleted/" + target_name
        if regen or filestamps.file_updated([source], target):
            shutil.copyfile(source, target)

def copy_static_keys():
    fixed_keydir = env.BASE_PATH + "/input/fixed_keys/"
    output_keydir = env.OUTPUT_BASE_PATH + "/keys/"
    os.makedirs(output_keydir, exist_ok=True)
    for filename in glob.glob(os.path.join(fixed_keydir, '*.*')):
        shutil.copy(filename, output_keydir)

if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Regenerate the zone files")
    (options, args) = parser.parse_args()
    generate_static_zone_entries()
    copy_static_keys()
    create_zone_files(options.regen)
