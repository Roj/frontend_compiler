#include "libs/lexical.h"
#include "libs/parser.h"
#include "libs/codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

//Assumption: first line is not empty. (sorry)
int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s source.c\n", argv[0]);
		return 1;
	}
	//It is necessary to print this out with no buffering, so if there are
	//any errors this message is shown first
	fprintf(stderr, "Parsing file %s\n", argv[1]);
	lexeme_t* start = process_file(argv[1]);
	NodeProgram* ast_root;
	if (parse(start, &ast_root)) {
		printf("Parsed correctly!\n");
	} else {
		printf("Does not fit the syntax!\n");
		return 2;
	}
	printf("Generating code\n");
	char filenameout[500];
	strcpy(filenameout, argv[1]);
	filenameout[strlen(filenameout)-1]='o';
	codegen(ast_root, filenameout);
	printf("Written file %s\nLink with: \ngcc %s -o %.*s -no-pie\n" , 
		filenameout, filenameout, (int) strlen(filenameout)-2, filenameout);
	return 0;
}
