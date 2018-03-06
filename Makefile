CFLAGS=-std=c99 -O1 -pedantic -Werror -Wtype -Wextra -Wall
CC=gcc

main: lexical.o
	$(CC) $(CFLAGS) lexical.o -o main


