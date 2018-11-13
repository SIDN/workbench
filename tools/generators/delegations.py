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
# This generator make the 'delegation' zones
#
# I.e. zones that are only served on one specific server,
# having delegations to the other servers (TODO: and itself?)
#

from workbench import dnsutil, env, filestamps, zonedata
import optparse
import os
#import shlex
#import shutil
#import subprocess

OUTPUT_FILE = "delegations.db"
BASE_ZONE = "delegations.wb.sidnlabs.nl."
ZONES_DIR = "output/uncompleted"
# each delegation will be served at firstlabel.sidnlabs.nl
SERVER_ZONE = "sidnlabs.nl."

delegation_servers = [
    "bind9",
    "bind10",
    "nsd",
    "nsd4",
    "powerdns",
    "knot",
    "yadifa"
]

def create_delegations_zonelist(name, depth):
    if depth <= 0:
        raise Exception("depth must be positive")
    zonelist = []
    zonelist.append(name)
    if depth > 1:
        for delname in delegation_servers:
            zonelist.extend(create_delegations_zonelist(delname + "." + name, depth-1))
    return zonelist
    
def create_zones(zones, regen):
    template_files = [ dnsutil.get_template_filename("basic_zone"),
                       dnsutil.get_template_filename("all_ns") ]
    zds = []
    # create the keys first
    for z in zones:
        # Check whether the key exists, if not, create
        keyfile = dnsutil.get_keyfile(z)
        dnsutil.check_create_key(z, keyfile)
        
    for z in zones:
        zonefile = ZONES_DIR + "/" + dnsutil.ufqdn(z)
        nsname = z.split(".")[0]

        zd = zonedata.ZoneData()
        zd.set("name", z)
        if nsname != "delegations":
            zd.add("primary_names", nsname)
        dnsutil.add_standard_sign_options(zd)
        zds.append(zd)

        if filestamps.file_updated(template_files, zonefile) or regen:
            with open(zonefile, "w") as out_file:
                dnsutil.add_template(out_file, "basic_zone", z, 3600)
                if nsname != "delegations":
                    # one NS at apex, start with firstlabel
                    out_file.write("        IN NS %s.%s\n" % (nsname, SERVER_ZONE))
                else:
                    # Add all nameservers for the top-level zone
                    dnsutil.add_template(out_file, "all_ns", z, 3600)
                for deleg in delegation_servers:
                    delname = deleg + "." + z
                    if delname in zones:
                        out_file.write("%s        IN NS %s.%s\n" % (delname, deleg, SERVER_ZONE))
        
    zonedata.write_zone_data(env.ZONE_DB_PATH + "/" + OUTPUT_FILE, zds)
    

if __name__=="__main__":
    parser = optparse.OptionParser()
    parser.add_option("-r", action="store_true", dest="regen",
                      help="Regenerate the zone files")
    (options, args) = parser.parse_args()
    zones = create_delegations_zonelist(BASE_ZONE, 4)
    create_zones(zones, options.regen)
