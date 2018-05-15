#include "libs/lexical.h"
#include "libs/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

//Assumption: first line is not empty. (sorry)
int main(int argc, char* argv[]) {
	char line[3000];
	if (argc == 3 && strcmp(argv[1], "--file") == 0) {
		//It is necessary to print this out with no buffering, so if there are
		//any errors this message is shown first
		fprintf(stderr, "Parsing file %s\n", argv[2]);
		lexeme_t* start = process_file(argv[2]);
		if (parse(start)) {
			printf("Parsed correctly!\n");
		} else {
			printf("Does not fit the syntax!\n");
		}
		return 0;
	}

	printf("Syntactical parser -- write code one block per line up to 3000 "
		"characters long\n");

	bool unfinished_comment = false;
	while (fgets(line, 2999, stdin) != NULL) {
		lexeme_t* start = process_string(line, &unfinished_comment);

		lexeme_t* current = start;

		while (current) {
			printf("%s ", lex2str(current));
			current = current->next;
		}
		printf("\n");
		if (parse(start)) {
			printf("Parsed correctly!\n");
		} else {
			printf("Does not fit the syntax!\n");
		}

		delete_lexemes(start);

	}
	return 0;
}
