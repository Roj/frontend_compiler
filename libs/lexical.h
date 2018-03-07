#ifndef __LEXICAL_H__
#define __LEXICAL_h__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef enum _lexeme_type_t { 
	UNDEF,
	NUMBER,
	IDENTIFIER,
	OP_PLUS,
	OP_MINUS,
	KW_IF,
	KW_FOR,
	KW_BLOCK_START,
	KW_BLOCK_END,
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

lexeme_t* process_string(char* str);

int get_error_count();
#endif
