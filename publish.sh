#!/bin/sh

DATE=`date -I`

check_rcode() {
    RCODE=$?
    if [ $RCODE -eq 0 ]; then
        echo -n "";
    else
        echo "Failed";
        exit $RCODE
    fi
}

# Generate config

#./scripts/generator.py configs/
#check_rcode

#
#
# Backups
#
#


# $1 = host, $2 = file, $3 = target file
backup_file() {
    echo "Backup on $1: $2 to $3"
    # Note: -n, only backup if we haven't called this script
    # today yet (usually when it fails we do not want to keep
    # the failing attempts)
    ssh -4 $1 cp -n $2 $3
    check_rcode
}

backup_file nsd /var/workbench/nsd.conf /var/workbench/nsd.conf.${DATE}
backup_file nsd4 /var/workbench/nsd.conf /var/workbench/nsd.conf.${DATE}
backup_file knot /var/workbench/knot.conf /var/workbench/knot.conf.${DATE}
backup_file bind9 /var/workbench/named.conf.workbench /var/workbench/named.conf.workbench.${DATE}
#backup_file bind10 /var/workbench/bind10_commands.txt /var/workbench/bind10_commands.txt.${DATE}
#backup_file yadifa /var/workbench/yadifad.conf /var/workbench/yadifad.conf.${DATE}
#backup_file bind10 /home/jelte/bind10_transfers.txt /home/jelte/bind10_transfers.txt.${DATE}
#backup_file bind10 /var/workbench/pdns_commands.txt /var/workbench/pdns_commands.txt.${DATE}

#
#
# Updated files
#
#

# Config files
# $1 = host, $2 = file, $3 = target file
update_file() {
    echo "Update config file on $1 source $2 target $3"
    scp -4 $2 $1:$3
    check_rcode
}

update_file nsd output/servers/nsd/nsd.conf /var/workbench/nsd.conf
update_file nsd output/servers/nsd/update.sh /var/workbench/update.sh
update_file nsd4 output/servers/nsd4/nsd4.conf /var/workbench/nsd.conf
update_file nsd4 output/servers/nsd4/update.sh /var/workbench/update.sh
update_file knot output/servers/knot/knot.conf /var/workbench/knot.conf
update_file knot output/servers/knot/update.sh /var/workbench/update.sh
update_file bind9 output/servers/bind9/bind9.conf /etc/bind/named.conf.workbench
update_file bind9 output/servers/bind9/update.sh /var/workbench/update.sh
#update_file bind10 output/servers/bind10/bind10.conf /var/workbench/bind10_commands.txt
#update_file bind10 output/servers/bind10/update.sh /var/workbench/update.sh
update_file powerdns output/servers/powerdns/update.sh /var/workbench/update.sh
#update_file yadifa output/servers/yadifa/yadifa.conf /var/workbench/yadifad.conf
update_file powerdns output/servers/powerdns/update.sh /var/workbench/update.sh
#update_file yadifa output/servers/yadifa/update.sh /var/workbench/update.sh

#update_file bind10 configs/bind10_transfers.txt /home/jelte/bind10_transfers.txt
# Powerdns updates itself (has nsd as supermaster)


# Zones
# Currently the only master is nsd, when we add more we need to
# change this. Probably port all to the generator tool
# By convention, all zones are placed in /var/workbench/zones
# and the update script in /var/workbench
#rsync -avz 
rsync -rz output/final/ nsd:/var/workbench/zones/
rsync -rz output/final/ nsd4:/var/workbench/zones/
rsync -rz output/final/ knot:/var/workbench/zones/
rsync -rz output/final/ bind9:/etc/bind/zones/
#rsync -rz output/final/ bind10:/var/workbench/zones
rsync -rz output/final/ powerdns:/var/workbench/zones/
#rsync -rz output/final/ yadifa:/var/workbench/zones/

#
#
# Apply the updates
#
#

apply_update() {
    echo "Apply the new configuration on $1"
    # On a couple of systems, it can be important to
    # have cwd set correctly
    ssh -4 $1 "cd /var/workbench; ./update.sh"
    # If things fail here, we continue anyway, in general
}

#apply_update nsd nsdc rebuild
#check_rcode

apply_update nsd
apply_update nsd4
apply_update knot
apply_update bind9
#apply_update yadifa
#apply_update bind10
apply_update powerdns
# bind10 and powerdns done by hand for now (those take too long
# with the current setup)

#apply_update nsd4 /var/workbench/nsd4/sbin/nsd-control reload
#apply_update knot sudo /etc/knot/knotc_update
#apply_update bind9 rndc reload
#apply_update bind10 /home/jelte/run_bindctl_commands.sh /home/jelte/bind10_commands.txt
#apply_update bind10 /home/jelte/run_bindctl_commands.sh /home/jelte/bind10_transfers.txt

#echo "Sleeping for a minute to let things settle"
#sleep 60;
#apply_update nsd nsdc notify

echo "All done"


