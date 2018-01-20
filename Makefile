###############################################################################
# NAME:		    makefile
#
# AUTHOR:	    Ethan D. Twardy
#
# DESCRIPTION:	    Makefile for the executable C code contained in set.c.
#
# CREATED:	    06/07/2017
#
# LAST EDITED:	    01/19/2018
###

CC=gcc
ifeq ($(MAKECMDGOALS),debug)
	CFLAGS = -g -std=c99 -O0 -Wall \
	-D CONFIG_DEBUG_SET -D CONFIG_TEST_LOG \
	-Wno-format
else
	CFLAGS = -std=c99 -Wall -O3
endif

.PHONY: debug clean

set: test.c

debug: set

clean:
	rm -rf *.dSYM
	rm -f *.o
	rm -f set

###############################################################################
