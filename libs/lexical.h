#ifndef __LEXICAL_H__
#define __LEXICAL_H__
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
	OP_NEQUALS,
	ASSIGN,
	ASSIGN_TYPE,
	KW_IF,
	KW_ELSE,
	KW_FOR,
	KW_TO,
	KW_DOWNTO,
	KW_DO,
	KW_VAR,
	KW_PROGRAM,
	KW_THEN,
	TYPE_INTEGER,
	COMMA,
	ARRAY_RANGE,
	BLOCK_START,
	BLOCK_END,
	PARENS_START,
	PARENS_END,
	BRACKET_START,
	BRACKET_END,
	STM_END,
	PROGRAM_END,
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

char* lex2str(lexeme_t* lex);
char* lextype2str(lexeme_type_t type);

lexeme_t* process_string(char* str, bool* unfinished_comment);

void delete_lexemes(lexeme_t* start);
int get_error_count();
#endif
