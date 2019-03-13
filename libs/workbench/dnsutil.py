
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
# Assorted DNS utility functions
#
from workbench import env, zonedata
import os
import shlex
import subprocess

def fqdn(zone):
    """
    Adds a root dot to the given string, if not present yet
    Returns the result
    """
    if not zone.endswith("."):
        return zone + "."
    else:
        return zone

def ufqdn(zone):
    """
    Removes the root dot from the given string, if present
    Returns the result
    """
    if zone.endswith("."):
        return zone[:-1]
    else:
        return zone

def is_direct_parent(parent, child):
    # assuming no ents
    # and no dots within labels
    # i.e. use with care ;)
    p = fqdn(parent).split(".")
    c = fqdn(child).split(".")
    
    # one exception for root; in which case split behaves a bit
    # differently
    if p == ["", ""]:
        del p[0]
    if c == ["", ""]:
        del c[0]

    if len(p) != len(c) - 1:
        return False
    del c[0]
    return p == c

def get_template_filename(name):
    return "%s/templates/%s.templ" % (env.INPUT_BASE_PATH, name)

def add_template(output_zone_file, name, origin=None, ttl=None):
    if origin is not None:
        output_zone_file.write("$ORIGIN %s\n" % str(origin))
    if ttl is not None:
        output_zone_file.write("$TTL %s\n" % str(ttl))
    with open(get_template_filename(name), "r") as input_file:
        for l in input_file.readlines():
            output_zone_file.write(l)

def add_standard_sign_options(zd):
    dname = zd.get("name")
    dname_u = ufqdn(dname)
    zd.set("signer_script", "ldns-signzone")
    zd.add("signer_params", "-o")
    zd.add("signer_params", dname)
    zd.add("signer_params", "-e")
    zd.add("signer_params", "20300101000000")
    zd.add("signer_params", "-f")
    zd.add("signer_params", "signed/" + dname_u)
    # Make this stdin too?
    zd.add("signer_params", "unsigned/" + dname_u)
    # TODO: generate keys here?
    # or move this out?
    # ...
    zd.add("signer_keys", dname_u)
    zd.add("signer_keys", dname_u + ".ksk")

def execute(cmd, cwd=None):
    print("[DEBUG] run command: %s" % cmd)
    cmdp = shlex.split(cmd)
    p = subprocess.Popen(cmdp, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=cwd)
    (stdout, stderr) = p.communicate()
    if p.returncode != 0:
        print(stderr)
        raise Exception("Command failed: " + cmd)
    return stdout

def get_keyfile(zone):
    zone = ufqdn(zone)
    return env.KEYS_DIR + "/" + zone + ".private"

def check_create_key(zone, keyfile):
    base_keyfile = keyfile[:-8]
    if not os.path.basename(base_keyfile).startswith("nods.") and not os.path.exists(keyfile):
        os.makedirs(os.path.dirname(keyfile), exist_ok=True)
        cmd = "ldns-keygen -k -r /dev/urandom -a RSASHA256 -b 1024 %s" % zone
        stdout = execute(cmd)
        basename = stdout.decode("utf-8").rstrip()
        
        #if (base_keyfile.startswith("nods.")):
        #    os.unlink(basename + ".ds")
        #else:
        os.rename(basename + ".ds", base_keyfile + ".ds")
        os.rename(basename + ".key", base_keyfile + ".key")
        os.rename(basename + ".private", base_keyfile + ".private")

def get_all_db_files():
    # TODO: this needs improving
    result = []
    for (dirpath, dirnames, filenames) in os.walk(env.ZONE_DB_PATH):
        for fn in filenames:
            dn = None
            if fn.endswith(".db"):
                result.append(env.ZONE_DB_PATH + "/" + fn)
    return result
    
def read_all_db_files():
    zone_data_list = []
    for dbf in get_all_db_files():
        zone_data_list.extend(zonedata.read_zone_data(dbf))
    return zone_data_list

