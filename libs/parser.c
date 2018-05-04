#include "parser.h"
#include <stdio.h>
#include <stdarg.h>
static lexeme_t* lex;
static int syntax_errors = 0;

void syntax_error(int num_options, ...) {
	va_list arguments; 
	va_start (arguments, num_options); 
	printf("Unexpected lexeme of type %s, expected ", lex2str(lex));
	for (int i = 0; i < num_options; i++) {
		printf("%s", lextype2str(va_arg(arguments, lexeme_type_t)));
		if (i < num_options-2)
			printf(", ");
		else if (i == num_options-2)
			printf(", or ");
	}
	printf(".\n");
	va_end(arguments);
	syntax_errors++;
}
void next_symb() {
	lex = lex->next;
}
void match(lexeme_type_t type) {
	if (lex->type == type) {
		next_symb();
	} else {
		syntax_error(1, type);
	}
}
//Returns true if the input was correctly processed
bool parse(lexeme_t* first_symbol) {
	syntax_errors = 0;
	lex = first_symbol;
	Grouping();
	return lex->type == EOI && syntax_errors == 0;
}
bool parse_unit(lexeme_t* fs, void(*Nonterminal)(void)) {
	syntax_errors = 0;
	lex = fs;
	Nonterminal();
	return lex->type == EOI && syntax_errors == 0;
}
void Grouping() {
	switch (lex->type) {
		case KW_IF:
			match(KW_IF);
			match(PARENS_START);
			Expression();
			match(PARENS_END);
			Block();
			Else();
			Grouping();
			break;
		case KW_FOR:
			match(KW_FOR);
			match(PARENS_START);
			Statement();
			match(STM_END);
			Expression();
			match(STM_END);
			Statement();
			match(PARENS_END);
			Block();
			Grouping();
			break;
		case IDENTIFIER: //first of Statement
			Statement();
			match(STM_END);
			Grouping();
			break;
		default:
			return;
		//	since it can be epsilon I don't think it throws synxerror on def
	}
}
void Statement() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			match(ASSIGN);
			Expression();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
}
void Else() {
	switch (lex->type) {
		case KW_ELSE:
			match(KW_ELSE);
			Block();
			break;
		default:
			return;
		//Same, no default.
	}
}
void Block() {
	switch (lex->type) {
		case BLOCK_START:
			match(BLOCK_START);
			Grouping();
			match(BLOCK_END);
			break;
		case IDENTIFIER: //first of Statement
			Statement();
			break;
		default:
			syntax_error(2, BLOCK_START, IDENTIFIER);
	}
}
void Expression() {
	switch (lex->type) {
		case OP_NOT:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			Term();
			ExpressionPrime();
			break;
		default:
			syntax_error(3, NUMBER, IDENTIFIER, PARENS_START);
	}
}
void ExpressionPrime() {
	switch (lex->type) {
		case OP_GT:
			match(OP_GT);
			Term();
			ExpressionPrime();
			break;
		case OP_GTE:
			match(OP_GTE);
			Term();
			ExpressionPrime();
			break;
		case OP_LT:
			match(OP_LT);
			Term();
			ExpressionPrime();
			break;
		case OP_LTE:
			match(OP_LTE);
			Term();
			ExpressionPrime();
			break;
		case OP_PLUS:
			match(OP_PLUS);
			Term();
			ExpressionPrime();
			break;
		case OP_MINUS:
			match(OP_MINUS);
			Term();
			ExpressionPrime();
			break;
		case OP_AND:
			match(OP_AND);
			Term();
			ExpressionPrime();
			break;
		case OP_OR:
			match(OP_OR);
			Term();
			ExpressionPrime();
			break;
		default:
			return;
		//No default error: ExpressionPrime -> epsilon
	}
}
void Term() {
	switch (lex->type) {
		case OP_NOT:
		case IDENTIFIER:
		case NUMBER:
		case PARENS_START:
			Factor();
			TermPrime();
			break;
		default:
			syntax_error(3, IDENTIFIER, NUMBER, PARENS_START);
	}
}
void TermPrime() {
	switch (lex->type) {
		case OP_MULTIPLY:
			match(OP_MULTIPLY);
			Factor();
			TermPrime();
			break;
		case OP_DIV:
			match(OP_DIV);
			Factor();
			TermPrime();
			break;
		default:
			return;
		//No default error: TermPrime -> epsilon
	}
}
void Factor() {
	switch (lex->type) {
		case OP_NOT:
			match(OP_NOT);
			Factor();
			break;
		case IDENTIFIER:
			match(IDENTIFIER);
			break;
		case NUMBER:
			match(NUMBER);
			break;
		case PARENS_START:
			match(PARENS_START);
			Expression();
			match(PARENS_END);
			break;
		default:
			syntax_error(3, IDENTIFIER, NUMBER, PARENS_START);
	}
}
