CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O0 -pedantic -Werror -Wextra -Wall -g -Q
CC=gcc

all: test lexical

lexical: libs/lexical.o lexical.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o lexical

parser: libs/lexical.o libs/parser.o parser.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o parser
	
test:  libs/lexical.o test/lexicaltest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testsuite

clean:
	rm -f libs/*.o test/*.o *.o main

.PHONY: clean
