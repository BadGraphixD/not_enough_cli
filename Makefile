CC = gcc
CFLAGS = -Wall -Werror

CDFLAGS = -pg -g
CRFLAGS = -O3

.PHONY: lib lib_debug lib_release clean

lib_debug: CFLAGS += $(CDFLAGS)
lib_debug: lib
lib_release: CFLAGS += $(CRFLAGS)
lib_release: lib

lib: not_enough_cli.c not_enough_cli.h
	mkdir -p bin
	$(CC) $(CFLAGS) -c not_enough_cli.c -o bin/lib.o

clean:
	rm -fr bin
