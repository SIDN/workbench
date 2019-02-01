#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "ERROR: You must be a root user. Quitting." 2>&1
exit 1
else
  echo "Publish sequence started:"
fi

DATE=`date -I`

check_rcode() {
    RCODE=$?
    if [ $RCODE -eq 0 ]; then
        echo "	Succes!";
    else
        echo "	Failed - aborting now";
        exit $RCODE
    fi
}

# Generate config
#
echo "- Generating zones"
#./create_zones.sh
echo "	(skipped as per design)"
check_rcode

# Generate config
#
echo "- Generating config"
#./create_configs.sh
echo "	(skipped as per design)"
check_rcode

#
# Backups
#

# $1 = host, $2 = file, $3 = target file
backup_file() {
    echo "- Backup $1 files: $2 to $3"
    # Note: -n, only backup if we haven't called this script
    # today yet (usually when it fails we do not want to keep
    # the failing attempts)
    mv -n $2 $3
    #check_rcode
}

# We don't do backups ;-)
#backup_file nsd4 /etc/nsd/workbench/nsd4.conf /etc/nsd/workbench/nsd4.conf.${DATE}
#backup_file knot /etc/knot/workbench/knot.conf /etc/knot/workbench/knot.conf.${DATE}
#backup_file bind9 /etc/bind/workbench/bind9.conf /etc/bind/workbench/bind9.conf.${DATE}
#backup_file yadifa /etc/yadifa/workbench/yadifad.conf /etc/yadifa/workbench/yadifad.conf.${DATE}
#backup_file powerdns /etc/powerdns/pdns.conf /etc/powerdns/pdns.conf.${DATE}

#
#
# Updated files
#
#

# Config files
# $1 = host, $2 = file, $3 = target file
update_file() {
    echo "- Update config file for $1 from source $2 to target $3"
    # TODO do we want the -n, or maybe not?
    cp $2 $3
    check_rcode
}

#update_file nsd output/servers/nsd/nsd.conf /var/workbench/nsd.conf
#update_file nsd output/servers/nsd/update.sh /var/workbench/update.sh
update_file nsd4 output/servers/nsd4/nsd4.conf /etc/nsd/workbench
update_file nsd4 output/servers/nsd4/update.sh /etc/nsd/workbench
update_file knot output/servers/knot/knot.conf /etc/knot/workbench
update_file knot output/servers/knot/update.sh /etc/knot/workbench
update_file bind9 output/servers/bind9/bind9.conf /etc/bind/workbench
update_file bind9 output/servers/bind9/update.sh /etc/bind/workbench
# No include trick for powerdns
update_file powerdns output/servers/powerdns/powerdns.conf /etc/powerdns/pdns.conf
update_file powerdns output/servers/powerdns/update.sh /etc/powerdns/workbench
update_file yadifa output/servers/yadifa/yadifa.conf /etc/yadifa/workbench
update_file yadifa output/servers/yadifa/update.sh /etc/yadifa/workbench
#update_file bind10 configs/bind10_transfers.txt /home/jelte/bind10_transfers.txt
# Powerdns has bind9 as supermaster

# TODO: fix description
#       add check_rcode

# Zones
# Currently the only master is bind9, when we add more we need to
# change this. Probably port all to the generator tool
# By convention, all zones are placed in /var/dns-workbench/zones (except for the two rfc3597 zones needed for Yadifa)
# and the update script in /etc/$nameserver
rm /var/dns-workbench/rfc3597zones/*
rm /var/dns-workbench/zones/*
cp -a output/final/* /var/dns-workbench/zones
check_rcode

#
#
# Apply the updates
#
#

apply_update() {
    echo "- Apply the new configuration on $1"
    bash $1/update.sh
    # If things fail here, we continue anyway, in general
}

# BIND first (others, like Yadifa and PowerDBS depend on it
apply_update /etc/bind/workbench
apply_update /etc/nsd/workbench
apply_update /etc/knot/workbench
apply_update /etc/powerdns/workbench

# Now comes a dirty trick...
# Wait a few seconds for bind to load
# Translate two zones to RFC3597-format
# Then reload Yadifa:
sleep 3
# TODO: fix the IP-addresses when going into production!!
dig +onesoa +unknownformat axfr types.wb.sidnlabs.nl @2a00:d78:0:712:94:198:159:39F > /var/dns-workbench/rfc3597zones/types.wb.sidnlabs.nl
dig +onesoa +unknownformat axfr types-signed.wb.sidnlabs.nl @2a00:d78:0:712:94:198:159:39F > /var/dns-workbench/rfc3597zones/types-signed.wb.sidnlabs.nl
sleep 1
apply_update /etc/yadifa/workbench

echo "- All done!"
