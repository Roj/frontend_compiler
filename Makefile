CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O0 -pedantic -Werror -Wextra -Wall -g `llvm-config --cflags` -Wno-discarded-qualifiers#-Q
CC=gcc
LD=g++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`

all: test lexical testlexical testparser parser codegen

lexical: libs/lexical.o lexical.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o lexical

parser: libs/lexical.o libs/parser.o libs/tree.o parser.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o parser
	
codegen.o: libs/lexical.o libs/parser.o libs/tree.o libs/codegen.o codegen.c
	$(CC) $(CFLAGS) $^ -c $<

codegen: libs/lexical.o libs/parser.o libs/tree.o libs/codegen.o codegen.o
	$(LD) $(LDFLAGS) $^ $(CLIBS) -o codegen

testlexical:  libs/lexical.o test/lexicaltest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testlexical

testparser: libs/lexical.o libs/parser.o libs/tree.o test/parsertest.o
	$(CC) $(CFLAGS) $^ $(CLIBS) -o testparser

clean:
	rm -f libs/*.o test/*.o *.o main testparser testlexical lexical parser

.PHONY: clean
