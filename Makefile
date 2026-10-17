CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic
LDFLAGS ?=

.PHONY: all test clean
all: xonitix

xonitix: main.c game.c terminal.c game.h terminal.h
	$(CC) $(CFLAGS) main.c game.c terminal.c -o $@ $(LDFLAGS)

test: tests/test_game.c game.c game.h
	$(CC) $(CFLAGS) tests/test_game.c game.c -o tests/test_game $(LDFLAGS)
	./tests/test_game

clean:
	rm -f xonitix tests/test_game
