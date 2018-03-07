#include "lexical.h"
#include <stdbool.h>
#include <assert.h>

#define IDENT_STRING_SIZE 20 //hardcoded for now

static int error_count = 0;

int get_error_count() {
	return error_count;
}

void syntax_error(char* input, int errorpos, char* msg) {
	/*We want to find the first character of the line,
	* so we can print an error message like this:
	SYNTAX ERROR
	if (0xFZA > j) {
	-------^
	etc. */
	int first_character = errorpos;
	while (first_character > 0 && input[first_character-1] != '\n') {
		first_character--;
	}
	printf("Syntax error:\n");	
	//Print the error line.
	for (int i = first_character; input[i] && input[i] != '\n'; i++) {
		printf("%c", input[i]);
	}
	printf("\n");
	//Print the arrow.
	for (int i = first_character; i < errorpos; i++) {
		printf("-");
	}
	printf("^\n");
	//Print the error.
	printf("%s\n", msg);

	error_count++;
}

bool is_white_space(char c) {
	return c == ' ';
}

//Transforms a possibly hex character to int.
//Returns -1 if it's not possible to convert.
int hexdig2dec(char c) {
	if ('0' <= c && c <= '9') 
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10; 
	return -1;
}
bool is_letter(char c) {
	return (
		('a' <= c && c <= 'z')
		|| ('A' <= c && c <= 'Z')
	);
}
bool is_alphanumeric(char c) {
	return (
		is_letter(c)
		|| ('0' <= c && c <= '9')
	);
}

lexeme_t* new_lexeme() {
	lexeme_t* new = calloc(1, sizeof(lexeme_t));
	assert(new != NULL);
	return new;
}

//Reads a decimal number, storing it in the lexeme_t*.
//Returns the position of the first non-numeric character
//in the input.
int read_dec_number(char* str, int i, lexeme_t* actual) {
	actual->type = NUMBER;
	while ('0' <= str[i] && str[i] <= '9') {
		actual->data.value = actual->data.value*10 + (str[i] - '0');
		i++;
	}
	return i;
}

//Reads a oct/hex number, storing it in the lexeme_t*.
//Returns the position of the first non-numeric character
//in the input.
int read_hex_oct_number(char* str, int i, lexeme_t* actual) {
	actual->type = NUMBER;
	i++;
	if (str[i] == 'x' || str[i] == 'X') {
		i++;
		while (is_alphanumeric(str[i])) {
			int value = hexdig2dec(str[i]);
			if (value == -1) {
				syntax_error(str, i, "Character not in hex digit range");
				return i;
			}
			actual->data.value = actual->data.value*16 + value;
			i++;
		}
	} else {
		while (is_alphanumeric(str[i])) {
			int value = str[i] - '0';
			if (value > 7) {
				syntax_error(str, i, "Character not in oct digit range");
				return i;
			}
			actual->data.value = actual->data.value*8 + value;
			i++;
		}
	}
	//Third case implicit: it was just a zero
	return i;
}

//Reads an identifier.
//Returns the position of the first non-alphanumeric character
//in the input.
int read_identifier(char* str, int i, lexeme_t* actual) {
	actual->type = IDENTIFIER;
	actual->data.name = malloc(sizeof(char) * (IDENT_STRING_SIZE + 1));
	assert(actual->data.name != NULL);
	int identlen = 0;
	while (is_alphanumeric(str[i])) {
		//Complexity is quite big rn but w/e, optimizations come later.
		if (identlen == IDENT_STRING_SIZE) {
			syntax_error(str, i, "Identifier name too long.");
			return i;
		}
		actual->data.name[identlen++] = str[i];
		i++;
	}
	actual->data.name[identlen] = '\0';
	return i;
}

//Decides if a lexeme-identifier is actually a keyword,
//and changes the type if appropiate.
void verify_change_keyword(lexeme_t* ident) {
	char* keywords_str[2] = {"if","for"};
	lexeme_type_t keywords[2] = {KW_IF, KW_FOR};
	for (int i = 0; i < 2; i++) {
		if (strcmp(keywords_str[i], ident->data.name) == 0) {
			free(ident->data.name);
			ident->type = keywords[i];
			return;
		}
	}
}

lexeme_t* process_string(char* str) {
	lexeme_t* start = new_lexeme();
	lexeme_t* actual = start;
	size_t i = 0;
	/*
	 * This might be written a little strange but it's to ensure that it 
	 * complies with being LL(1).
	 */
	while (str[i]) {
		if ('1' <= str[i] && str[i] <= '9') {
			i = read_dec_number(str, i, actual);
		} else if ('0' == str[i]) {
			i = read_hex_oct_number(str, i, actual);
		} else if (is_letter(str[i])) {
			i = read_identifier(str, i, actual);
		} else {
			if (actual->type != UNDEF) {
				//Identifiers can actually be keywords, so let's check that.
				if (actual->type == IDENTIFIER)
					verify_change_keyword(actual);
				//Finish the previous token and create a new one.
				lexeme_t* new = new_lexeme();
				actual->next = new;
				actual = new;
			}
			i++;
		}
	}
	if (actual->type == UNDEF) {
		actual->type = EOI;
	} else {
		if (actual->type == IDENTIFIER)
			verify_change_keyword(actual);
		lexeme_t* end = new_lexeme();
		end->type = EOI;
		actual->next = end;
		actual = end;
	}
	return start;
}
