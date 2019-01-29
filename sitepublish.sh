#!/bin/bash
#
# This script does not create the site (there's a seperate script for that)!
# It only puts it in place.
#

if [[ $EUID -ne 0 ]]; then
  echo "ERROR: You must be a root user. Quitting." 2>&1
exit 1
else
  echo "Publish sequence started:"
fi

check_rcode() {
    RCODE=$?
    if [ $RCODE -eq 0 ]; then
        echo "	Succes!";
    else
        echo "	Failed - aborting now";
        exit $RCODE
    fi
}

echo "Remove the old version"
rm -rf /var/www/html/*
check_rcode
echo "Install new version"
cp -a site/* /var/www/html/
check_rcode

echo "- All done!"
