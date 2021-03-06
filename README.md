![SIDN Labs Logo](https://workbench.sidnlabs.nl/assets/img/sidnlabs_logo.png)

This is the workbench generator code v4 (2020 - apex special signer).

This version changes the way the 'bad-dnssec' tree is generated; it now
places the errors in the zones themselves, instead of at the delegation
points. This version also solves the 'bogus' problem with unknownalgorithm.

See the Changelog file for more details, and all changes.

The directory layout is as follows:

- /ext        External tools, scripts, paths, freeform layout.
- /libs       Internal libs for shared code between scripts
- /generators Scripts that generate the data, each subdirectory is one 'type' of generator.
- /input      Static input data
- /output     Generated and copied data is placed below this directory

Prerequisites:

- [ldns-dns-workbench](https://github.com/SIDN/ldns-dns-workbench): a fork
  of ldns that contains the special zone signer, as well as some other
  modifications that are used by the DNS workbench generator tools)
- named-compilezone: Optional, to make the output zones prettier. On most
  systems, this tool is provided by the bind9utils package.

The tools (ldns-sign-special, ldns-3597, ldns-keygen with TwoCents
support, and the optional named-compilezone) need to be in your PATH; you
do not need to install them to your system; so building ldns-dns-workbench
(with examples!) and adding <build_dir>/example to your PATH should
suffice.

We have a convenience script that recreates everything:

    ./from_scratch.sh

This rebuilds all zones, all configuration, and front-end website; if you
only need to do part of this, please check the commands it calls.

Als look in /base_configs to find out what prerequisites exist for the
system (we use Ubuntu 18.04).

Steps to create additional content:

- Write a generator (to get entries into the db), or add to an existing one
- Run the generator
    * only adds/updates entries in the db
- Run the content-creator
    * takes db entries, and depending on them, generates 'stuff'
- Run the zone-creator
    * takes the db entries, and copies/updates/adds zone files
- Run the zone-completer
    * for each zone that has a DS record, add it to the parent zone,
      if any
    * also updates SERIAL if necessary
- Run the signer(s)
    * takes the db entries and zone files, and signs or copies them
- Run the config creator(s)
    * takes the db entries and generates config files
- publish the zones/update the configs
    * dependent on target...

The zones will have a number of duplicates, for two reasons; it keeps
the separation of the steps more clean, and it makes it easier to
update only parts of them.

The zone files go through a couple of stages (and dirs):
- input/
    if there is any input (optional)
- uncompleted/
    before the completer is done
- unsigned/
    before they are signed
- signed/
    after they are signed (copied if they are not signed at all)
- final/
    if no processor_script, simply copied, otherwise the processor
    should place them here

GENERATORS
----------

Generators take some (local) input (or not), and generate entries
for the zone db. Every zone that ends up in the system should have
an entry in a file in the zone_db directory. Generators are free
to hard-code them or use their own input files (conventionally under
input/<generator name>/). The output file is conventionally called
<generator_name>.db, and placed in the output/zone_db directory.

The db files are simple textual representations (currently in CSV),
as written by the ZoneData class. In practice, this means generators
make ZoneData objects, and write them to the files. All later tools in
the chain should take these ZoneData objects as inputs to generate
other data and zones.

See the static_zone generator for a simple example.


INPUT
-----

Most raw zone data in found in the input/ directory; this directory
contains full zones that can be used for input in some generators,
but it also contains a template/ directory with zone chunks (such as
a list with all the NS records). By convention, neither contains a
hostname part, this will have to be filled in by the generator (@ is
used, so origin or previous name will be used).

OUTPUT
------

A lot of (intermediate) output is generated. In the end there are
two directories that matter:
output/signed/ and output/servers

output/signed contains all zones ('unsigned' zones are simply copied)
for all servers. For convenience we don't differentiate between zones
that are and are not served on each server at this point, but simply
share this directory with all servers.

output/servers contains a subdirectory for each server, named by their
shortname. These contain two files; a .conf and an update.sh. In most
cases, the .conf is the configuration file, and update.sh simply
contains the command to reload it. In certain exceptional cases
(like bind10), the .conf contains a list of commands, and the script
pipes them to the system (in this example, through bindctl).


