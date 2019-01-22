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
    cp -n $2 $3
    #check_rcode
}

backup_file nsd4 /etc/nsd/workbench/nsd4.conf /etc/nsd/workbench/nsd4.conf.${DATE}
#backup_file knot /var/workbench/knot.conf /var/workbench/knot.conf.${DATE}
backup_file bind9 /etc/bind/workbench/bind9.conf /etc/bind/workbench/bind9.conf.${DATE}
#backup_file yadifa /var/workbench/yadifad.conf /var/workbench/yadifad.conf.${DATE}
# TODO: where is powerdns?!

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
#update_file knot output/servers/knot/knot.conf /var/workbench/knot.conf
#update_file knot output/servers/knot/update.sh /var/workbench/update.sh
update_file bind9 output/servers/bind9/bind9.conf /etc/bind/workbench
update_file bind9 output/servers/bind9/update.sh /etc/bind/workbench
#update_file powerdns output/servers/powerdns/update.sh /var/workbench/update.sh
#update_file powerdns output/servers/powerdns/update.sh /var/workbench/update.sh
#update_file yadifa output/servers/yadifa/yadifa.conf /var/workbench/yadifad.conf
#update_file yadifa output/servers/yadifa/update.sh /var/workbench/update.sh

#update_file bind10 configs/bind10_transfers.txt /home/jelte/bind10_transfers.txt
# Powerdns updates itself (has nsd as supermaster)


# TODO: fix description
#       add check_rcode

# Zones
# Currently the only master is nsd, when we add more we need to
# change this. Probably port all to the generator tool
# By convention, all zones are placed in /var/workbench/zones
# and the update script in /var/workbench
cp -a output/final/* /var/dns-workbench/zones
check_rcode
#rsync -rz output/final/ knot:/var/workbench/zones/
#rsync -rz output/final/ bind9:/etc/bind/zones/
#rsync -rz output/final/ powerdns:/var/workbench/zones/
#rsync -rz output/final/ yadifa:/var/workbench/zones/

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

apply_update /etc/nsd/workbench
#apply_update /etc/knot/workbench
apply_update /etc/bind/workbench
#apply_update /etc/yadifa/workbench
#apply_update /etc/powerdns/workbench

echo "- All done!"
