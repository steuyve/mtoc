CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99

mtoc: src/mtoc.c
	$(CC) -o bin/$@ $< $(CLFAGS)

debug: src/mtoc.c
	$(CC) -g -o bin/$@ $< $(CLFAGS)

profile: src/mtoc.c
	$(CC) -pg -o bin/$@ $< $(CLFAGS)
