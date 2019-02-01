$TTL 60

@		86400	IN SOA	bind9.sidnlabs.nl. hostmaster.sidnlabs.nl. (
					0
					3600
					600
					1814400
					3600 )
;@			NS	nsd.sidnlabs.nl.
@			NS	nsd4.sidnlabs.nl.
@			NS	bind9.sidnlabs.nl.
@			NS	knot.sidnlabs.nl.
@			NS	powerdns.sidnlabs.nl.
@			NS	yadifa.sidnlabs.nl.

; type 1
a01			A	0.0.0.0
a02			A	255.255.255.255
a			A	198.51.100.53 ; Address of web host
type1			A	198.51.100.53 ; Address of web host
www			A	198.51.100.53 ; Address of web host

; type 2
; see NS records at top of file

; type 3
;md			MD	maildestination
;type3			MD	maildestination
md			TXT	"Removed because BIND9 refuses to load it"
type3		TXT	"Removed because BIND9 refuses to load it"


; type 4
;mf			MF	mailforwarder
;type4			MF	mailforwarder
mf			TXT	"Removed because BIND9 refuses to load it"
type4			TXT	"Removed because BIND9 refuses to load it"

; type 5
cname01			CNAME	cname-target.
cname02			CNAME	cname-target
;cname03		CNAME	.
cname			CNAME	www
type5			CNAME	www

; type 6
; see SOA record at top of file

; type 7
;mb			MB	mailbox
;type7			MB	mailbox
mb			TXT	"Removed because NSD4 refuses to load it"
type7			TXT	"Removed because NSD4 refuses to load it"


; type 8
mg01			MG	mgmname
mg02			MG	.
mg			MG	mailgroupmember
type8			MG	mailgroupmember

; type 9
mr01			MR	mrname
mr02			MR	.
mr			MR	mailrename
type9			MR	mailrename

; type 10
; NULL RRs are not allowed in master files per RFC1035.

; type 11
wks01			WKS	10.0.0.1 tcp telnet ftp 0 1 2
wks02			WKS	10.0.0.1 udp domain 0 1 2
; ldns does not appear to accept this one (TODO)
;wks03			WKS	10.0.0.2 tcp 65535
wks			WKS	10.0.0.1 tcp telnet ftp 0 1 2
type11			WKS	10.0.0.1 tcp telnet ftp 0 1 2

; type 12
ptr01			PTR	@
type12			PTR	@

; type 13
hinfo01			HINFO	"Generic PC clone" "NetBSD-1.4"
hinfo02			HINFO	PC NetBSD
hinfo			HINFO	"Generic PC clone" "MyOS"
type13			HINFO	"Generic PC clone" "MyOS"

; type 14
minfo01			MINFO	. .
minfo			MINFO	boxmaster.foo.bar. mailbox.there.
type14			MINFO	boxmaster.foo.bar. mailbox.there.

; type 15
mx01			MX	10 mail
;mx02			MX	10 .
mx				MX	10 maildoesntwork
type15			MX	10 maildoesntwork

; type 16
txt01			TXT	"foo"
txt02			TXT	"foo" "bar"
txt03			TXT	foo
txt04			TXT	foo bar
txt05			TXT	"foo bar"
txt06			TXT	"foo\032bar"
txt07			TXT	foo\032bar
txt08			TXT	"foo\010bar"
txt09			TXT	foo\010bar
txt10			TXT	foo\ bar
txt11			TXT	"\"foo\""
txt12			TXT	\"foo\"
txt				TXT	\"Just some\" \" text\"
type16			TXT	\"Just some\" \" text\"

; type 17
rp01			RP	mbox-dname txt-dname
rp02			RP	. .
rp				RP	. txt
type17			RP	. txt

; type 18
afsdb01			AFSDB	0 hostname
afsdb02			AFSDB	65535 .
afsdb			AFSDB	12345 afsnode
type18			AFSDB	12345 afsnode

; type 19
x25			X25	"3033033033"
type19			X25	"3033033033"

; type 20
isdn01			ISDN	"isdn-address"
isdn02			ISDN	"isdn-address" "subaddress"
isdn03			ISDN	isdn-address
isdn04			ISDN	isdn-address subaddress
isdn			ISDN	"isdn-address" "subaddress"
type20			ISDN	"isdn-address" "subaddress"

; type 21
rt01			RT	0 intermediate-host
rt02			RT	65535 .
rt				RT	10	rthost
type21			RT	10	rthost

; type 22
nsap01			NSAP	(
	0x47.0005.80.005a00.0000.0001.e133.ffffff000161.00 )
type22			NSAP	(
	0x47.0005.80.005a00.0000.0001.e133.ffffff000161.00 )

;this one make dig axfr hang? (probably on nsd side)
;nsap02			NSAP	0x

nsap			NSAP	0x012345

; type 23
; TODO: this one makes dig (but not drill) spew garbage, not sure
; if NSD or dig.
;nsap-ptr		TYPE23	\# 12 087369646e6c616273026e6c ; sidnlabs.nl

; type 24
;sig01			SIG	NXT 1 3 ( 3600 20000102030405
;				19961211100908 2143 foo.nil.
;				MxFcby9k/yvedMfQgKzhH5er0Mu/vILz45I
;				kskceFGgiWCn/GxHhai6VAuHAoNUz4YoU1t
;				VfSCSqQYn6//11U6Nld80jEeC8aTrO+KKmCaY= )

; type 25
;key01			KEY	512 ( 255 1 AQMFD5raczCJHViKtLYhWGz8hMY
;				9UGRuniJDBzC7w0aRyzWZriO6i2odGWWQVucZqKV
;				sENW91IOW4vqudngPZsY3GvQ/xVA8/7pyFj6b7Esg
;				a60zyGW6LFe9r8n6paHrlG5ojqf0BaqHT+8= )

; type 26
px			PX	10	map822 mapx400
type26			PX	10	map822 mapx400

; type 27
gpos			TYPE27	\# 18 05 32 33 2e 36 37 05 32 33 2e 36 37 05 32 33 2e 36 37
type27			TYPE27	\# 18 05 32 33 2e 36 37 05 32 33 2e 36 37 05 32 33 2e 36 37

; type 28
aaaa			AAAA	2001:db8:53::beef
type28			AAAA	2001:db8:53::beef

; type 29
loc01			LOC	60 9 N 24 39 E 10 20 2000 20
loc02			LOC 	60 09 00.000 N 24 39 00.000 E 10.00m 20.00m (
				  2000.00m 20.00m )
loc				LOC	60 9 N 24 39 E 10 20 2000 20
type29			LOC	60 9 N 24 39 E 10 20 2000 20

; type 30
;nxt01			NXT	a.secure.nil. ( NS SOA MX RRSIG KEY LOC NXT )
;nxt02			NXT	. NXT NSAP-PTR
;nxt03			NXT	. 1
;nxt04			NXT	. 127

; type 33
srv01			SRV 0 0 0 foo.
srv02			SRV 65535 65535 65535  old-slow-box
srv				SRV 65535 65535 65535  old-slow-box
type33			SRV 65535 65535 65535  old-slow-box

; type 35
naptr01			NAPTR   0 0 "" "" "" .
naptr02			NAPTR   65535 65535 blurgh blorf blllbb foo.
naptr02			NAPTR   65535 65535 "blurgh" "blorf" "blllbb" foo.
naptr			NAPTR	100 100 "s" "http+l@r" "" naptr.replacement
type35			NAPTR	100 100 "s" "http+l@r" "" naptr.replacement

; type 36
kx01			KX	10 kdc
kx02			KX	10 .
kx			KX	10 kx
type36			KX	10 kx

; type 37
cert01			CERT	65534 65535 254 (
				MxFcby9k/yvedMfQgKzhH5er0Mu/vILz45I
				kskceFGgiWCn/GxHhai6VAuHAoNUz4YoU1t
				VfSCSqQYn6//11U6Nld80jEeC8aTrO+KKmCaY= )
cert			CERT	65534 65535 254 (
				MxFcby9k/yvedMfQgKzhH5er0Mu/vILz45I
				kskceFGgiWCn/GxHhai6VAuHAoNUz4YoU1t
				VfSCSqQYn6//11U6Nld80jEeC8aTrO+KKmCaY= )
type37			CERT	65534 65535 254 (
				MxFcby9k/yvedMfQgKzhH5er0Mu/vILz45I
				kskceFGgiWCn/GxHhai6VAuHAoNUz4YoU1t
				VfSCSqQYn6//11U6Nld80jEeC8aTrO+KKmCaY= )

; type 38
; TODO: this one makes dig (but not drill) spew garbage, not sure
; if NSD or dig.
;a6			TYPE38	\# 29  00 ffffffffffffffffffffffffffffffff 087369646e6c616273026e6c ; sidnlabs.nl


; type 39
dname01			DNAME	dname-target.
dname02			DNAME	dname-target
dname03			DNAME	.
dname			DNAME	dname-target.
type39			DNAME	dname-target.

; type 41
; OPT is a meta-type and should never occur in master files.

; type 46
; we sign it ourselves
;rrsig01			RRSIG	NSEC 1 3 ( 3600 20000102030405
;				19961211100908 2143 foo.nil.
;				MxFcby9k/yvedMfQgKzhH5er0Mu/vILz45I
;				kskceFGgiWCn/GxHhai6VAuHAoNUz4YoU1t
;				VfSCSqQYn6//11U6Nld80jEeC8aTrO+KKmCaY= )

; type 47
; we sign it ourselves
;nsec01			NSEC	a.secure.nil. ( NS SOA MX RRSIG DNSKEY LOC NSEC )
;nsec02			NSEC	. NSEC NSAP-PTR
;nsec03			NSEC	. TYPE1
;nsec04			NSEC	. TYPE127

; type 48
; we sign it ourselves
;dnskey01		DNSKEY	512 ( 255 1 AQMFD5raczCJHViKtLYhWGz8hMY
;				9UGRuniJDBzC7w0aRyzWZriO6i2odGWWQVucZqKV
;				sENW91IOW4vqudngPZsY3GvQ/xVA8/7pyFj6b7Esg
;				a60zyGW6LFe9r8n6paHrlG5ojqf0BaqHT+8= )

; type 49
dhcid			DHCID	AAIBY2/AuCccgoJbsaxcQc9TUapptP69lOjxfNuVAA2kjEA=
type49			DHCID	AAIBY2/AuCccgoJbsaxcQc9TUapptP69lOjxfNuVAA2kjEA=

; type 50
; nsec3 will be handles by a different zone

; type 51
; nsec3param will be handles by a different zone

; type 52
; note that owner name is not according to spec.
tlsa			TYPE52 \# 67 01 01 02 92 00 3b a3 49 42 dc 74 15 2e 2f 2c 40 8d 29 ec a5 a5 20 e7 f2 e0 6b b9 44 f4 dc a3 46 ba f6 3c 1b 17 76 15 d4 66 f6 c4 b7 1c 21 6a 50 29 2b d5 8c 9e bd d2 f7 4e 38 fe 51 ff d4 8c 43 32 6c bc
type52			TYPE52 \# 67 01 01 02 92 00 3b a3 49 42 dc 74 15 2e 2f 2c 40 8d 29 ec a5 a5 20 e7 f2 e0 6b b9 44 f4 dc a3 46 ba f6 3c 1b 17 76 15 d4 66 f6 c4 b7 1c 21 6a 50 29 2b d5 8c 9e bd d2 f7 4e 38 fe 51 ff d4 8c 43 32 6c bc

; type 53
; no type 53

; type 54
; no type 54

; type 55
; TODO: this one makes dig (but not drill) spew garbage, not sure
; if NSD or dig.
;hip				TYPE55 \# 149 02200100107B1A74DF365639CC39F1D57803010001B771CA136E4AEB5CE44333C53B3D2C13C22243851FC708BCCE29F7E2EB5787B5F56CCAD34F8223ACC10904DDB56B2EC4A6D6232F3B50EA094F0914B3B941BBE529AF582C36BBADEFDAF2ADAF9B4911906F5B2522603C615272B880EC8FB930CC6EE39C444DAA75B1678F005A4B2499D1DA5433F805C7A5AD3237ACC5DD5C5E43

; type 56
; text value: "This zone is experimental."
ninfo			TYPE56	\# 27 1a54686973207a6f6e65206973206578706572696d656e74616c2e
type56			TYPE56	\# 27 1a54686973207a6f6e65206973206578706572696d656e74616c2e

; type 57
; RKEY values taken from our DNSKEY
; AwEAAde1PJyYjnR2R0RmzDuiYKRh/ldkv0znVOYwfjsHZNLg0ahLI+UsvghBmimoUSGa9d6Ckd3dodbHYxUpjFYsJfdeq+qimYFjrG8bUA2BD2uJMag1/QG7DTUp3jHaV0Q13r/829QEl0sjrLIBxC7wSlqu0ydfYz5VX7X0A8i1vDm9
rkey			TYPE57	\# 136 0000010803010001D7B53C9C988E7476474466CC3BA260A461FE5764BF4CE754E6307E3B0764D2E0D1A84B23E52CBE08419A29A851219AF5DE8291DDDDA1D6C76315298C562C25F75EABEAA2998163AC6F1B500D810F6B8931A835FD01BB0D3529DE31DA574435DEBFFCDBD404974B23ACB201C42EF04A5AAED3275F633E555FB5F403C8B5BC39BD
type57			TYPE57	\# 136 0000010803010001D7B53C9C988E7476474466CC3BA260A461FE5764BF4CE754E6307E3B0764D2E0D1A84B23E52CBE08419A29A851219AF5DE8291DDDDA1D6C76315298C562C25F75EABEAA2998163AC6F1B500D810F6B8931A835FD01BB0D3529DE31DA574435DEBFFCDBD404974B23ACB201C42EF04A5AAED3275F633E555FB5F403C8B5BC39BD

; type 58
; value:  talink TALINK h0.example.com. h2.example.com.
talink			TYPE58	\# 32 026830076578616d706c6503636f6d00026831076578616d706c6503636f6d00
type58			TYPE58	\# 32 026830076578616d706c6503636f6d00026831076578616d706c6503636f6d00

; type 59
; source: signed.workbench.sidnlabs.nl.	IN	DS	64690 8 2 86632f83494b1d7037e72949fd6cd8689c5daaf4df1e5d7e6ef3ba28ece1e3c8
cds				TYPE59	\# 36 fcb2080286632f83494b1d7037e72949fd6cd8689c5daaf4df1e5d7e6ef3ba28ece1e3c8
type59			TYPE59	\# 36 fcb2080286632f83494b1d7037e72949fd6cd8689c5daaf4df1e5d7e6ef3ba28ece1e3c8

; nothing from 60 to 98

; type 99
spf			SPF	"v=spf1 +mx a:colo.example.com/28 -all"
type99			SPF	"v=spf1 +mx a:colo.example.com/28 -all"

; types 100-103 are reserved by IANA

; type 104
nid			TYPE104 \# 10 00 0a 00 94 01 98 01 52 01 69
type104			TYPE104 \# 10 00 0a 00 94 01 98 01 52 01 69

; type 105
l32			TYPE105 \# 6 00 0a c0 00 02 01
type105			TYPE105 \# 6 00 0a c0 00 02 01

; type 106
l64			TYPE106 \# 10 00 0a 2a 00 0d 78 00 04 05 03
type106			TYPE106 \# 10 00 0a 2a 00 0d 78 00 04 05 03

; type 107
lp			TYPE107 \# 27 00 0a 0b 6c 36 34 2d 73 75 62 6e 65 74 31 07 65 78 61 6d 70 6c 65 03 63 6f 6d 00
type107			TYPE107 \# 27 00 0a 0b 6c 36 34 2d 73 75 62 6e 65 74 31 07 65 78 61 6d 70 6c 65 03 63 6f 6d 00

; type 108, see http://tools.ietf.org/html/draft-jabley-dnsext-eui48-eui64-rrtypes-07
eui48			TYPE108 \# 6 00 00 5e 00 53 2a
type108			TYPE108 \# 6 00 00 5e 00 53 2a

; type 109, see http://tools.ietf.org/html/draft-jabley-dnsext-eui48-eui64-rrtypes-07
eui64			TYPE109 \# 8 00 00 5e ef 10 00 00 2a
type109			TYPE109 \# 8 00 00 5e ef 10 00 00 2a
