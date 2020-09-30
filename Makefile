CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99

mtoc: src/mtoc.c
	$(CC) -o bin/$@ $< $(CFLAGS)

debug: src/mtoc.c
	$(CC) -g -o bin/$@ $< $(CFLAGS)

profile: src/mtoc.c
	$(CC) -pg -o bin/$@ $< $(CFLAGS)
