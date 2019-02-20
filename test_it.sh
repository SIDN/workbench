#!/bin/bash
#
# TODO: This will be a script that checks if all zones work
#       on all name servers where tey are supposed to work
#	It does *nothing* just yet.
#

# TODO:
#
#	- Test SOA (serial) of all zones on all name servers where the zone is supposed to be present
#		(apexcname will only be on powerdns probably)
#		(serials must match)
#	- Check the number of zones to see if they are all there (how?)
#	- Check if all name servers allow AXFR (with and without TSIG)
#	- Check if (and how) NSID, id.server, hostname.bind, hostname.pdns and version.bind/version.pdns respond
#	- Run commands whereever possible to see if zones are loaded with warnings or errors
#		(Or try to scan the logging for stuff, maybe)
#	- What else?

# This, maybe?
#
# for a in $(ls /var/dns-workbench/zones/); do dig +short SOA @resolv1.sidn.nl  $a; done
#

# This, maybe too?
#nsd-control stats | egrep '(zone.master|zone.slave)'
#	zone.master=292
#	zone.slave=4
#knotc stats
#	server.zone-count = 296

