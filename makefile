################################################################################
# NAME: makefile
# AUTHOR: Ethan D. Twardy
# DESCRIPTION: makefile for the executable C code contained in set.c
# CREATED: 06/07/17
# LAST EDITED: 06/07/17
################################################################################

P = set
OBJECTS = set.c
CFLAGS = -std=c99 -Wall -O3
LDLIBS=
CC=gcc

$(P):
	$(CC) $(CFLAGS) -o $(P) $(OBJECTS) $(LDLIBS)

.PHONY: debug clean

CFLAGS_DEBUG = -g -std=c99 -O0 -Wall -D CONFIG_DEBUG_SET

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(P) $(OBJECTS) $(LDLIBS)

clean:
	rm -rf *.c~
	rm -rf *.h~
	rm -rf makefile~
