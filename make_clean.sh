#!/bin/sh
#
# Clean it up to an original state
# (quick-n-dirty, might need some extra checks and improvements)
#

rm -rf ./output
rm -rf ./site

find . -type f -name "*~" | xargs rm

#
# Run as root
#
# sqlite3 /var/lib/powerdns/pdns.sqlite3 < powerdns_clean.sql
