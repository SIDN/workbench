/* Run as root: */
/* sqlite3 /var/lib/powerdns/pdns.sqlite3 < powerdns_types_slave.sql */
/* BIND does not send notifies btw, but we can live without that */
INSERT INTO domains (name, master, type) VALUES ('types.wb.sidnlabs.nl', '94.198.159.39, 2a00:d78:0:712:94:198:159:39', 'SLAVE');
INSERT INTO domains (name, master, type) VALUES ('types-signed.wb.sidnlabs.nl', '94.198.159.39, 2a00:d78:0:712:94:198:159:39', 'SLAVE');

INSERT INTO domains (name, master, type) VALUES ('nsec3-opt-out.wb.sidnlabs.nl', '94.198.159.39, 2a00:d78:0:712:94:198:159:39', 'SLAVE');
INSERT INTO domains (name, master, type) VALUES ('wildcards-nsec3.wb.sidnlabs.nl', '94.198.159.39, 2a00:d78:0:712:94:198:159:39', 'SLAVE');
