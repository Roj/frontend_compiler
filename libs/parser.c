#include "parser.h"
#include <stdio.h>
static lexeme_t* lex;
static int syntax_errors = 0;
//Forward declarations.
void Grouping();
void Statement();
void Block();
void Else();
void Expression();
void ExpressionPrime();
void Term();
void TermPrime();
void Factor();

void syntax_error() {
	printf("Unexpected lexeme of type %s\n", lex2str(lex));
	syntax_errors++;
}
void next_symb() {
	lex = lex->next;
}
void match(lexeme_type_t type) {
	if (lex->type == type) {
		next_symb();
	} else {
		syntax_error();
	}
}
//Returns true if the input was correctly processed
bool parse(lexeme_t* first_symbol) {
	lex = first_symbol;
	Grouping();
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
			syntax_error();
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
			syntax_error();
	}
}
void Expression() {
	switch (lex->type) {
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			Term();
			ExpressionPrime();
			break;
		default:
			syntax_error();
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
		default:
			return;
		//No default error: ExpressionPrime -> epsilon
	}
}
void Term() {
	switch (lex->type) {
		case IDENTIFIER:
		case NUMBER:
		case PARENS_START:
			Factor();
			TermPrime();
			break;
		default:
			syntax_error();
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
			syntax_error();
	}
}
