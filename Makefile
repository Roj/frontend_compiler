CLIBS=-lcheck -lsubunit -pthread -pthread -lrt -lm -lsubunit
CFLAGS=-std=c99 -O1 -pedantic -Werror -Wextra -Wall  
CC=gcc

main: lexical.o
	$(CC) $(CFLAGS) lexical.o $(CLIBS) -o main

clean:
	rm -f *.o main

.PHONY: clean
