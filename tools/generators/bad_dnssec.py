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
import os
import shlex
import shutil
import subprocess

print(env.ZONE_DB_PATH)

OUTPUT_FILE = "bad_dnssec.db"
# TODO: needed?
INPUT_DIR = env.INPUT_BASE_PATH + "/bad_dnssec"
BASE_ZONE = "bad-dnssec.wb.sidnlabs.nl."
ZONES_DIR = "output/uncompleted"
SIGNER = env.EXT_TOOLS_PATH + "/ldns-sign-special/ldns-sign-special"

def add_standard_sign_options(zd):
    dname = zd.get("name")
    dname_u = dnsutil.ufqdn(dname)
    zd.set("signer_script", SIGNER)
    zd.add("signer_params", "-o")
    zd.add("signer_params", dname)
    zd.add("signer_params", "-e")
    zd.add("signer_params", "20300101000000")
    zd.add("signer_params", "-f")
    zd.add("signer_params", "signed/" + dname_u)
    # Make this stdin too?
    zd.add("signer_params", "unsigned/" + dname_u)
    zd.add("signer_keys", dname_u)
    zd.add("signer_keys", dname_u + ".ksk")

def generate_bad_dnssec_zone_entry(zds, zone_name):
    zd = zonedata.ZoneData()
    dname = dnsutil.fqdn(zone_name)
    zd.set("name", dname)
    add_standard_sign_options(zd)
    zds.append(zd)
    
def create_zone_files(regen = False):
    # in essence, simply copy them to uncompleted
    # unless regen is specified, only copy if original is updated
    source_file = INPUT_DIR + "/types.wb.sidnlabs.nl"
    target_file1 = env.OUTPUT_BASE_PATH + "/uncompleted/types.wb.sidnlabs.nl"
    target_file2 = env.OUTPUT_BASE_PATH + "/uncompleted/types-signed.wb.sidnlabs.nl"
    if regen or filestamps.file_updated(target_file1):
        shutil.copyfile(source_file, target_file1);
    if regen or filestamps.file_updated(target_file2):
        shutil.copyfile(source_file, target_file2);
    # Copy all files in auto
    for af in list_files():
        # Create an entry for each file in auto
        autofiles = list_files()
        source_file = af[1]
        target_file = env.OUTPUT_BASE_PATH + "/uncompleted/" + af[0]
        if filestamps.file_updated([source_file], target_file):
            shutil.copyfile(source_file, target_file)

#
#
# to about here
#
#
bad_dnssec_tree_delegations = [
    "ok",
    "nods",
    "bogussig",
    "sigexpired",
    "signotincepted",
    "unknownalgorithm"
]

def create_bad_dnssec_tree_zonelist(name, depth):
    if depth <= 0:
        raise Exception("depth must be positive")
    zonelist = []
    zonelist.append(name)
    if depth > 1:
        for bdtd in bad_dnssec_tree_delegations:
            zonelist.extend(create_bad_dnssec_tree_zonelist(bdtd + "." + name, depth - 1))
    return zonelist

def create_zone(zone, zonefile):
    zone = dnsutil.fqdn(zone)
    #print("  [create_zone] zone: %s - zonefile: %s" %(zone,zonefile))
    # create tempfile and write zone data to is
    with open(zonefile, "w") as out:
        # TODO: serial... (and other values)
        # hmz must add all secondaries here as well
        dnsutil.add_template(out, "basic_zone", zone, 3600)
        dnsutil.add_template(out, "all_ns", zone, 3600)
        
        for delegation in bad_dnssec_tree_delegations:
            delname = delegation + "." + zone
            dsfile = env.KEYS_DIR + "/" + delname + "ds"
            # print("  check for nods-delegation or " + dsfile)
            if os.path.exists(dsfile):
                dnsutil.add_template(out, "all_ns", delname, 3600)
                if delname.startswith("nods."):
                    os.unlink(dsfile)
                # TODO: second check necessary?
                #if os.path.exists(dsfile):
                #    with open(dsfile, "r") as infile:
                #        for line in infile:
                #            out.write(line)
                #    out.write("\n")

def create_bad_dnssec_tree(regen):
    zone_list = create_bad_dnssec_tree_zonelist(BASE_ZONE, 4)
    # For each of those, check whether key exists, and generate
    # the zone if necessary
    for zone in zone_list:
        # Check whether the key exists, if not, create
        keyfile = dnsutil.get_keyfile(zone)
        dnsutil.check_create_key(zone, keyfile)

    used_templates =\
        [dnsutil.get_template_filename(t) for t in ["all_ns", "basic_zone" ]]

    zds = []
    for zone in zone_list:
        # Check whether the key exists, if not, create
        keyfile = dnsutil.get_keyfile(zone)
        zonefile = ZONES_DIR + "/" +  dnsutil.ufqdn(zone)
        # Create the zone
        if filestamps.file_updated(used_templates + [keyfile], zonefile):
            print("[bad_dnssec] Creating zone " + zone)
            create_zone(zone, zonefile)
        # Add entry to db
        generate_bad_dnssec_zone_entry(zds, zone)
    zonedata.write_zone_data(env.ZONE_DB_PATH + "/" + OUTPUT_FILE, zds)


if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Regenerate the zone files")
    (options, args) = parser.parse_args()
    create_bad_dnssec_tree(options.regen)
