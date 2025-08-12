CC = gcc
CFLAGS = -Wall -Werror
LD = ld

CDFLAGS = -pg -g
CRFLAGS = -O3

.PHONY: lib lib_debug lib_release clean

lib_debug: CFLAGS += $(CDFLAGS)
lib_debug: lib
lib_release: CFLAGS += $(CRFLAGS)
lib_release: lib

lib: bin/not_enough_cli.o
	mkdir -p bin
	$(LD) -r $^ -o bin/lib.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

bin/not_enough_cli.o: not_enough_cli.c

clean:
	rm -fr bin
