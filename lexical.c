#include "libs/lexical.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
	char line[100];
	printf("Lexical analyser -- write code line by line up to 100 "
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
		delete_lexemes(start);
	}
	return 0;
}
