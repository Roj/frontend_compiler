CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O0 -pedantic -Werror -Wextra -Wall -g
CC=gcc

test: libs/lexical.o test/lexicaltest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testsuite

clean:
	rm -f libs/*.o test/*.o *.o main

.PHONY: clean
