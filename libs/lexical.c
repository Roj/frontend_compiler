#include "lexical.h"
#include <stdbool.h>
#include <assert.h>

#define IDENT_STRING_SIZE 20 //hardcoded for now
#define LITERAL_SIZE 30
static int error_count = 0;
static int line_num = 0;

char* lex_str_map[EOI+1] = {
	"UNDEF",
	"NUMBER",
	"IDENTIFIER",
	"LITERAL",
	"OP_PLUS",
	"OP_MINUS",
	"OP_DIV",
	"OP_INTDIV",
	"OP_MOD",
	"OP_MULTIPLY",
	"OP_GT",
	"OP_LT",
	"OP_GTE",
	"OP_LTE",
	"OP_NOT",
	"OP_AND",
	"OP_OR",
	"OP_EQUALS",
	"OP_NEQUALS",
	"ASSIGN",
	"ASSIGN_TYPE",
	"KW_IF",
	"KW_ELSE",
	"KW_FOR",
	"KW_TO",
	"KW_DOWNTO",
	"KW_DO",
	"KW_VAR",
	"KW_CONST",
	"KW_PROGRAM",
	"KW_THEN",
	"KW_OF",
	"KW_FUNCTION",
	"KW_PROCEDURE",
	"KW_FORWARD",
	"KW_WHILE",
	"KW_ARRAY",
	"TYPE_INTEGER",
	"COMMA",
	"ARRAY_RANGE",
	"BLOCK_START",
	"BLOCK_END",
	"PARENS_START",
	"PARENS_END",
	"BRACKET_START",
	"BRACKET_END",
	"STM_END",
	"PROGRAM_END",
	"EOI"
};
int get_error_count() {
	return error_count;
}

void lexical_error(char* input, int errorpos, char* msg) {
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
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
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

bool is_operator(char c) {
	return (
		c == '+'
		|| c == '-'
		|| c == '/'
		|| c == '*'
		|| c == '<'
		|| c == '>'
		|| c == '='
		|| c == '!'
		|| c == ':'
		|| c == '{'
		|| c == ','
		|| c == '.'
		|| c == '$'
		|| c == '&'
	);
}

bool is_grouping(char c) {
	return (
		c == '('
		|| c == ')'
		|| c == '['
		|| c == ']'
	);
}

lexeme_t* new_lexeme() {
	lexeme_t* new = calloc(1, sizeof(lexeme_t));
	new->line_num = line_num;
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
	if (is_letter(str[i]))
		lexical_error(str, i, "Unexpected letter");
	return i;
}

//Reads a hex number. Returns position of first non-alphanumeric character.
int read_hex_number(char* str, int i, lexeme_t* actual) {
	while (is_alphanumeric(str[i])) {
		int value = hexdig2dec(str[i]);
		if (value == -1) {
			lexical_error(str, i, "Character not in hex digit range");
			return i;
		}
		actual->data.value = actual->data.value*16 + value;
		i++;
	}
	return i;
}
//Reads an oct number starting with 0. Returns position of first non-alphnum.
int read_oct_number(char* str, int i, lexeme_t* actual) {
	while (is_alphanumeric(str[i])) {
		int value = str[i] - '0';
		if (value > 7) {
			lexical_error(str, i, "Character not in oct digit range");
			return i;
		}
		actual->data.value = actual->data.value*8 + value;
		i++;
	}
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
	while (is_alphanumeric(str[i]) || str[i] == '_') {
		//Complexity is quite big rn but w/e, optimizations come later.
		if (identlen == IDENT_STRING_SIZE) {
			lexical_error(str, i, "Identifier name too long.");
			return i;
		}
		actual->data.name[identlen++] = str[i];
		i++;
	}
	actual->data.name[identlen] = '\0';
	return i;
}
int read_literal(char* str, int i, lexeme_t* actual) {
	actual->type = LITERAL;
	actual->data.name = malloc(sizeof(char) * (LITERAL_SIZE +1));
	assert(actual->data.name != NULL);
	int litlen = 0;
	for (i = i+1; str[i] && str[i] != '\''; i++) {
		if (litlen == LITERAL_SIZE) {
			lexical_error(str, i, "Literal too long.");
			return i;
		}
		actual->data.name[litlen++] = str[i];
	}
	actual->data.name[litlen] = '\0';
	if (!str[i]) {
		lexical_error(str, i, "Unexpected end of input while reading string literal");
		return i;
	}
	return i+1;
}

int find_comment_end(char* str, int i, bool* unfinished_comment) {
	while (str[i]) {
		if (str[i] == '}')
			break;
		i++;
	}
	//Move the pointer to the next position after the comment
	if (str[i]) {
		*unfinished_comment = false;
		i++;
	}

	return i;
}
int read_op_or_comment(char* str, int i, lexeme_t* actual, bool* unfinished_comment) {
	switch (str[i]) {
		case '.':
			actual->type = PROGRAM_END;
			if (str[++i] == '.') {
				actual->type = ARRAY_RANGE;
				i++;
			}
			return i;
		case ',':
			actual->type = COMMA;
			return i+1;
		case '{':
			*unfinished_comment = true;
			return find_comment_end(str, i, unfinished_comment);
		case ':':
			actual->type = ASSIGN_TYPE;
			if (str[++i] == '=') {
				actual->type = ASSIGN;
				i++;
			}
			return i;
		case '+':
			actual->type = OP_PLUS;
			return i+1;
		case '-':
			actual->type = OP_MINUS;
			return i+1;
		case '>':
			actual->type = OP_GT;
			if (str[++i] == '=') {
				actual->type = OP_GTE;
				i++;
			}
			return i;
		case '<':
			actual->type = OP_LT;
			i++;
			if (str[i] == '=') {
				actual->type = OP_LTE;
				i++;
			} else if (str[i] == '>') {
				actual->type = OP_NEQUALS;
				i++;
			}
			return i;
		case '=':
			actual->type = OP_EQUALS;
			return i+1;
		case '/':
			actual->type = OP_DIV;
			return i+1;
		case '*':
			actual->type = OP_MULTIPLY;
			return i+1;
		case '$':
			actual->type = NUMBER;
			return read_hex_number(str, i+1, actual);
		case '&':
			actual->type = NUMBER;
			return read_oct_number(str, i+1, actual);
	}
	return i;
}
int read_grouping(char* str, int i, lexeme_t* actual) {
	switch (str[i]) {
		case '(':
			actual->type = PARENS_START;
			break;
		case ')':
			actual->type = PARENS_END;
			break;
		case '[':
			actual->type = BRACKET_START;
			break;
		case ']':
			actual->type = BRACKET_END;
			break;
	}
	return i+1;
}
//Decides if a lexeme-identifier is actually a keyword,
//and changes the type if appropiate.
void verify_change_keyword(lexeme_t* ident) {
	char* keywords_str[24] = {"if","for", "while", "then", "else", "to", "downto",
		"do", "begin", "end", "var", "const", "of", "integer", 
		"program", "function", "procedure", "forward",
		"not", "and", "or",
		"mod", "div",
		"array"
		};
	lexeme_type_t keywords[24] = {
		KW_IF, KW_FOR, KW_WHILE, KW_THEN, KW_ELSE, KW_TO, KW_DOWNTO, 
		KW_DO, BLOCK_START, BLOCK_END, KW_VAR, KW_CONST, KW_OF, TYPE_INTEGER,
		KW_PROGRAM, KW_FUNCTION, KW_PROCEDURE, KW_FORWARD,
		OP_NOT, OP_AND, OP_OR, 
		OP_MOD, OP_INTDIV, KW_ARRAY};
	for (int i = 0; i < 24; i++) {
		if (strcmp(keywords_str[i], ident->data.name) == 0) {
			free(ident->data.name);
			ident->type = keywords[i];
			return;
		}
	}
}
//Creates a new lexeme and stores it on *lex,
//keeping the list structure.
void finish_lexeme(lexeme_t** lex) {
	lexeme_t* new = new_lexeme();
	(*lex)->next = new;
	*lex = new;
}
void check_finish_previous_token(lexeme_t** actual) {
	//Identifiers can actually be keywords, so let's check that.
	if ((*actual)->type == IDENTIFIER) {
		verify_change_keyword(*actual);
		finish_lexeme(actual);
	} else if ((*actual)->type == NUMBER) {
		finish_lexeme(actual);
	}
}

/*Processes a string, encoding it to internal representation
 * lexemes. Holds a state in unfinished_comment, since this function
 * can be called multiple times for the same program, and comments
 * may span many function calls 
 * */
lexeme_t* process_string(char* str, bool* unfinished_comment) {
	lexeme_t* start = new_lexeme();
	lexeme_t* actual = start;
	size_t i = 0;
	/*
	 * This might be written a little strange but it's to ensure that it 
	 * complies with being LL(1).
	 */
	while (str[i]) {
		if (*unfinished_comment) {
			i = find_comment_end(str, i, unfinished_comment);
		} else if ('0' <= str[i] && str[i] <= '9') {
			i = read_dec_number(str, i, actual);
		} else if (is_letter(str[i])) {
			i = read_identifier(str, i, actual);
		} else if (is_operator(str[i])) {
			//Operators can close numbers and identifiers,
			//so let's check it.
			check_finish_previous_token(&actual);
			i = read_op_or_comment(str, i, actual, unfinished_comment);
			//Border case, since it may be a comment.
			if (actual->type != UNDEF) {
				//We already know it's an operator, no need to wait for the
				//next input to finish the lex.
				finish_lexeme(&actual);
			}
		} else if (is_grouping(str[i])) {
			check_finish_previous_token(&actual);
			i = read_grouping(str, i, actual);
			//Same idea.
			finish_lexeme(&actual);
		} else if (is_white_space(str[i])) {
			//Whitespace. Let's check if we can close the prev token.
			check_finish_previous_token(&actual);
			i++;
		} else if (str[i] == ';') {
			check_finish_previous_token(&actual);
			actual->type = STM_END;
			finish_lexeme(&actual);
			i++;
		} else if (str[i] == '\'') {
			i = read_literal(str, i, actual);
			finish_lexeme(&actual);
		} else {
			lexical_error(str, i, "Unexpected token");	
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

void delete_lexemes(lexeme_t* current) {
	while (current) {
		if (current->type == IDENTIFIER)
			free(current->data.name);
		lexeme_t* old = current;
		current = current->next;
		free(old);
	}
}
lexeme_t* process_file(char* filename) {
	FILE* file = fopen(filename, "r");
	assert(file != NULL);
	lexeme_t* previous_last = NULL;
	lexeme_t* start = NULL;
	char line[3000];
	bool unfinished_comment = false;
	while (fgets(line, 2999, file) != NULL) {
		line_num++;
		if (! start) {
			start = process_string(line, &unfinished_comment);
			previous_last = start;
			continue;
		}
		for (lexeme_t* lex = previous_last; lex->type != EOI; lex = lex->next) {
			previous_last = lex;
		}
		delete_lexemes(previous_last->next);
		previous_last->next = process_string(line, &unfinished_comment);
	}
	line_num = 0;
	return start;
}

char* lex2str(lexeme_t* lex) {
	if (lex->type <= EOI)
		return lex_str_map[lex->type];
	printf("Not found for type %d\n", lex->type);
	return lex_str_map[0];
}

char* lextype2str(lexeme_type_t type) {
	return lex_str_map[type];
}
