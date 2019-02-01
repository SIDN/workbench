#!/bin/bash

# Fetch the latest configs from the production environment
# And put them in Git.
#
# Together with stuff fromt he config_builder these make a working whole.
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
