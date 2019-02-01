 /* Run as root: */
 /* sqlite3 /var/lib/powerdns/pdns.sqlite3 < powerdns_supermaster.sql */
 DELETE FROM supermasters;
 VACUUM;
 INSERT INTO supermasters values ('2a00:d78:0:712:94:198:159:39', 'bind9.sidnlabs.nl', 'workbench');
 INSERT INTO supermasters values ('94.198.159.39', 'bind9.sidnlabs.nl', 'workbench');
  