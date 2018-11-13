# Should you have ldns in a non-standard location, you
# can fill it in here (or submit a full automake patch ;))
CFLAGS=-I/home/jelte/repos/ext/ldns/
LDFLAGS=-g -O2 -L/home/jelte/repos/ext/ldns -L/home/jelte/repos/ext/ldns/lib -lpcap -lldns -lcrypto -ldl
CC=gcc

all: ldns-sign-special

ldns-sign-special:	ldns-sign-special.c
	${CC} -o ldns-sign-special ldns-sign-special.c ${CFLAGS} ${LDFLAGS}

clean:
	rm ldns-sign-special
