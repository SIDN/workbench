
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
# Standard environment settings for the workbench code
# and a bit of global data?
#
# Currently mostly hardcoded, this should be generated/environed
#

import os

BASE_PATH=os.getcwd()

INPUT_BASE_PATH="%s/input" % BASE_PATH
OUTPUT_BASE_PATH="%s/output" % BASE_PATH
ZONE_DB_PATH="%s/output/zone_db" % BASE_PATH
FILESTAMPS_FILE="%s/output/.fstamps" % BASE_PATH
EXT_TOOLS_PATH="%s/ext" % BASE_PATH
DOMAIN="wb.sidnlabs.nl."
KEYS_DIR = "%s/output/keys" % BASE_PATH


# Address list
SERVERS = {
    # TODO: nsd is master - hier goed naar kijken, voordat je hem uitcommentarieerd
    "nsd": "94.198.159.25",
    "nsd4": "94.198.159.33",
    "bind9": "94.198.159.39",
    "knot": "94.198.159.27",
    "powerdns": "94.198.159.26",
    "yadifa": "94.198.159.28",
}
