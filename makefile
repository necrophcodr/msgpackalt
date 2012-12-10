#!/usr/bin/make
####################################################################################################
# MSGPACK MAKE SCRIPT
####################################################################################################
# note that the msgpack lib will not cross-compile under windows, so some tests will fail to compile
#   so compile with MSVC or run make with -k to compile the remainder
#
CC=gcc
CFLAGS=-Wall -pedantic -I . -Wno-long-long -O3
#MSGPACK0=../msgpack-0.5.4

all: msgpackalt.so

msgpackalt.so : msgpackalt.c msgpackalt.h
	$(CC) $(CFLAGS) -shared -o $@ $< -Wall -pedantic

#### MSGPACK0 is incompatible with GCC in the default build
#tests/%0 : tests/%0.c
#	$(CC) $(CFLAGS) -I $(MSGPACK0)/include -L $(MSGPACK0)/lib	 $<	-o $@	-lmsgpack -lws2_32
