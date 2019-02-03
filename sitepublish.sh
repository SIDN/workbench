#!/bin/bash
#
# This script does not create the site (there's a seperate script for that)!
# It only puts it in place.
#

if [[ $EUID -ne 0 ]]; then
  echo "ERROR: You must be a root user. Quitting." 2>&1
exit 1
else
  echo "- Publish sequence started:"
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

#echo "- Cleanup *~"
#find . -type f -name "*~" | xargs rm
#check_rcode

echo "- Remove the old version"
rm -rf /var/www/html/*
check_rcode
rm -rf /var/cache/lighttpd/compress/*
check_rcode
echo "- Install new version"
cp -a site/* /var/www/html/
check_rcode
# Needed for the Directory Listing pages:
ln -s /var/www/html/assets/favicon.ico /var/www/html/
check_rcode

echo "- Refresh the unsigend and signed zones (types.html refers to them)"
mkdir /var/www/html/zones
mkdir /var/www/html/delegationzones
mkdir /var/www/html/typeszones
mkdir /var/www/html/badzones
for a in $(ls /var/dns-workbench/zones/); do ln -s /var/dns-workbench/zones/$a /var/www/html/zones/$a.txt;done
for a in $(ls /var/dns-workbench/zones/types*.wb.sidnlabs.nl | awk -F\/ '{print $5}'); do ln -s /var/dns-workbench/zones/$a /var/www/html/typeszones/$a.txt;done
# please note, we do want to include delegations.wb.sidnlabs.nl itself here, so *dele... instead of *.dele...
for a in $(ls /var/dns-workbench/zones/*delegations.wb.sidnlabs.nl | awk -F\/ '{print $5}'); do ln -s /var/dns-workbench/zones/$a /var/www/html/delegationzones/$a.txt;done
for a in $(ls /var/dns-workbench/zones/*bad-dnssec.wb.sidnlabs.nl | awk -F\/ '{print $5}'); do ln -s /var/dns-workbench/zones/$a /var/www/html/badzones/$a.txt;done
#ln -s /var/dns-workbench/zones/types-signed.wb.sidnlabs.nl /var/www/html/types-signed.wb.sidnlabs.nl.txt
#ln -s /var/dns-workbench/zones/types.wb.sidnlabs.nl /var/www/html/types.wb.sidnlabs.nl.txt

echo "- All done!"
echo "(if it doesn't work; did you do ./create_site.sh first ???)"
