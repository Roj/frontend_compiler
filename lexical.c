#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef enum _lexeme_type_t { 
	NUMBER,
	IDENTIFIER,
	OP_PLUS,
	OP_MINUS,
	KW_IF,
	KW_FOR,
	KW_BLOCK_START,
	KW_BLOCK_END

} lexeme_type_t;

typedef struct lexeme {
	lexeme_type_t type;
	union data {
		char* name;
		double dvalue;
		int value;
	} data;
} lexeme_t;

int main() { 
	char* input = "1 + 2";

	return 0;
}
