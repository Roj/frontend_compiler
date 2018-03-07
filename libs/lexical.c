#include "lexical.h"
#include <stdbool.h>
#include <assert.h>

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
bool is_alphanumeric(char c) {
	return (
		('a' <= c && c <= 'z')
		|| ('0' <= c && c <= '9')
		|| ('A' <= c && c <= 'Z')
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
		} else {
			if (actual->type != UNDEF) {
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
		lexeme_t* end = new_lexeme();
		end->type = EOI;
		actual->next = end;
		actual = end;
	}
	return start;
}
