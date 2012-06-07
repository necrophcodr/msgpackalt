#!/usr/bin/make
####################################################################################################
# MSGPACK MAKE SCRIPT
####################################################################################################
# note that the msgpack lib will not cross-compile under windows, so some tests will fail to compile
#   so compile with MSVC or run make with -k to compile the remainder
#
CC=gcc
CFLAGS=-Wall -pedantic -I . -Wno-long-long -O3

all: msgpackalt.so $(basename $(wildcard tests/*.c))

msgpackalt.so : msgpackalt.c msgpackalt.h
	$(CC) -shared -o $@ $< -Wall -pedantic

tests/test : tests/test.c
tests/speed_test : tests/speed_test.c

tests/%0 : tests/%0.c
	$(CC) $(CFLAGS)	 $<	-o $@	-lmsgpack
