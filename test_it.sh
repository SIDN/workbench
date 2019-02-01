#!/bin/bash
#
# TODO: This wil be a script that checks if all zones work
#       on all name servers where tey are supposed to work
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


