CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O0 -pedantic -Werror -Wextra -Wall -g -Q
CC=gcc

all: test lexical
lexical: libs/*.o lexical.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o lexical
test: libs/*.o test/*.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testsuite

clean:
	rm -f libs/*.o test/*.o *.o main

.PHONY: clean
