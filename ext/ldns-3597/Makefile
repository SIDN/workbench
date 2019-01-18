# Should you have ldns in a non-standard location, you
# can fill it in here (or submit a full automake patch ;))

#CFLAGS=-I/home/jelte/repos/ext/ldns/
#LDFLAGS=-g -O2 -L/home/jelte/repos/ext/ldns -L/home/jelte/repos/ext/ldns/lib -lldns

CFLAGS= -Wall -g -O2
LDFLAGS= -lm -lldns
CC=gcc

all: ldns-3597

ldns-3597:	ldns-3597.c
	${CC} -o ldns-3597 ldns-3597.c ${CFLAGS} ${LDFLAGS}

clean:
	rm ldns-3597
