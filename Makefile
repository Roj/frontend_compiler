CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O0 -pedantic -Werror -Wextra -Wall -g #-Q
CC=gcc

all: test lexical testlexical testparser parser

lexical: libs/lexical.o lexical.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o lexical

parser: libs/lexical.o libs/parser.o parser.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o parser
	
testlexical:  libs/lexical.o test/lexicaltest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testlexical

testparser: libs/lexical.o libs/parser.o test/parsertest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testparser

clean:
	rm -f libs/*.o test/*.o *.o main testparser testlexical lexical parser

.PHONY: clean
