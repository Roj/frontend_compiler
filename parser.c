#include "libs/lexical.h"
#include "libs/parser.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
	char line[100];
	printf("Syntactical parser -- write code one block per line up to 100 "
		"characters long\n");

	bool unfinished_comment = false;
	while (fgets(line, 99, stdin) != NULL) {
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
