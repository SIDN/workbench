 /* Run as root: */
 /* sqlite3 /var/lib/powerdns/pdns.sqlite3 < powerdns_clean.sql */
 /* Clean it up! */
 /* comments and cryptokeys not really needed */
 DELETE FROM comments;
 DELETE FROM domainmetadata;
 DELETE FROM records;
 DELETE FROM tsigkeys;
 DELETE FROM cryptokeys;
 DELETE FROM domains;
 DELETE FROM supermasters;
 VACUUM;
 /* INSERT INTO supermasters values ('2a00:d78:0:712:94:198:159:39', 'bind9.sidnlabs.nl', 'workbench'); */
 /* Note: creating the tables was part of installing the package. Look there if you need to */
 /* Or see: https://github.com/PowerDNS/pdns/blob/rel/auth-4.1.x/modules/gsqlite3backend/schema.sqlite3.sql */
   