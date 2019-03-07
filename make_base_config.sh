#!/bin/bash
#
# Internally used script - you probably don't need it
#

if [[ $EUID -ne 0 ]]; then
  echo "ERROR: You must be a root user. Quitting." 2>&1
exit 1

# Fetch the latest configs from our production environment at https://workbench.sidnlabs.nl/
# And put them in Git as config-samples.
#
# Together with stuff fromt he config_builder these would make a working whole.
#

# Lighttpd
cp -a /etc/lighttpd/lighttpd.conf  base_configs/lighttpd/

# Network
cp -a /etc/netplan/50-cloud-init.yaml base_configs/netplan/

# BIND9
cp -a /etc/bind/named.conf base_configs/bind9/
cp -a /etc/bind/named.conf.options base_configs/bind9/
# Apparmor for BIND
cp -a /etc/apparmor.d/local/dns-workbench  base_configs/apparmor/
cp -a /etc/apparmor.d/usr.sbin.named  base_configs/apparmor/usr.sbin.named

# Knot
cp -a /etc/knot/knot.conf base_configs/knot/

# NSD4
cp -a /etc/nsd/nsd.conf base_configs/nsd4/

# PowerDNS
cp -a /etc/powerdns/pdns.conf base_configs/powerdns/

# YADIFA
cp -a /etc/yadifa/yadifad.conf base_configs/yadifa/
# the beast crashes just a little too often...
cp -a /etc/systemd/system/yadifa.service.d/restart.conf base_configs/yadifa/
cp -a /lib/systemd/system/yadifa.service base_configs/yadifa/ 
