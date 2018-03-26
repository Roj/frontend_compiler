#ifndef __LEXICAL_H__
#define __LEXICAL_h__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum _lexeme_type_t { 
	UNDEF,
	NUMBER,
	IDENTIFIER,
	OP_PLUS,
	OP_MINUS,
	OP_DIV,
	OP_MULTIPLY,
	OP_GT,
	OP_LT,
	OP_GTE,
	OP_LTE,
	OP_NOT,
	OP_AND,
	OP_OR,
	OP_EQUALS,
	ASSIGN,
	KW_IF,
	KW_FOR,
	BLOCK_START,
	BLOCK_END,
	PARENS_START,
	PARENS_END,
	BRACKET_START,
	BRACKET_END,
	EOI
} lexeme_type_t;

typedef struct lexeme {
	lexeme_type_t type;
	union data {
		char* name;
		double dvalue;
		int value;
	} data;
	struct lexeme* next;
} lexeme_t;

lexeme_t* process_string(char* str, bool* unfinished_comment);

void delete_lexemes(lexeme_t* start);
int get_error_count();
#endif
