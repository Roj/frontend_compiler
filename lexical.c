#include "libs/lexical.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct lex_str_match {
	lexeme_type_t type;
	char* name;
} lex_str_map_t;

lex_str_map_t lex_str_map[25] = {
	{UNDEF, "UNDEF"},
	{NUMBER, "NUMBER"},
	{IDENTIFIER, "IDENTIFIER"},
	{OP_PLUS, "OP_PLUS"},
	{OP_MINUS, "OP_MINUS"},
	{OP_DIV, "OP_DIV"},
	{OP_MULTIPLY, "OP_MULTIPLY"},
	{OP_EQUALS, "OP_EQUALS"},
	{ASSIGN, "ASSIGN"},
	{OP_GT, "OP_GT"},
	{OP_GTE, "OP_GTE"},
	{OP_LT, "OP_LT"},
	{OP_LTE, "OP_LTE"},
	{OP_NOT, "OP_NOT"},
	{OP_OR, "OP_OR"},
	{OP_AND, "OP_AND"},
	{KW_IF, "KW_IF"},
	{KW_FOR, "KW_FOR"},
	{BLOCK_START, "BLOCK_START"},
	{BLOCK_END, "BLOCK_END"},
	{PARENS_START, "PARENS_START"},
	{PARENS_END, "PARENS_END"},
	{BRACKET_START, "BRACKET_START"},
	{BRACKET_END, "BRACKET_END"},
	{EOI, "EOI"}
};

char* lex2str(lexeme_t* lex) {
	for(int i = 0; i < 26; i++) {
		if (lex_str_map[i].type == lex->type)
			return lex_str_map[i].name;
	}
	printf("Not found for type %d\n", lex->type);
	return lex_str_map[0].name;
}


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
