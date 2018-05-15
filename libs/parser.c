#include "parser.h"
#include <stdio.h>
#include <stdarg.h>
#define syntax_error(...) syntax_error_sp(__func__, __FILE__, __LINE__, __VA_ARGS__)
#define match(...) match_sp(__func__, __FILE__, __LINE__, __VA_ARGS__)
static lexeme_t* lex;
static int syntax_errors = 0;

void syntax_error_sp(const char* func, char* file, 
	int line, int num_options, ...) {
	va_list arguments; 
	va_start (arguments, num_options); 
	printf("Line %d: unexpected lexeme of type %s, expected ", 
		lex->line_num, lex2str(lex));
	for (int i = 0; i < num_options; i++) {
		//TODO: add a function that converts type to actual symbol
		//e.g. PARENS_START-> (
		printf("%s", lextype2str(va_arg(arguments, lexeme_type_t)));
		if (i < num_options-2)
			printf(", ");
		else if (i == num_options-2)
			printf(", or ");
	}
	printf(" in non-terminal %s, at %s:%d.\n", func, file, line);
	va_end(arguments);
	syntax_errors++;
}
void next_symb() {
	lex = lex->next;
}
void match_sp(const char* func, char* file, int line, lexeme_type_t type) {
	if (lex->type == type) {
		next_symb();
	} else {
		syntax_error_sp(func, file, line, 1, type);
	}
}
//Returns true if the input was correctly processed
bool parse(lexeme_t* first_symbol) {
	syntax_errors = 0;
	lex = first_symbol;
	Program();
	if (lex->type != EOI) {
		fprintf(stderr, "End is not EOI, is %s in line %d.\n", 
			lex2str(lex), lex->line_num);
	}
	return lex->type == EOI && syntax_errors == 0;
}
bool parse_unit(lexeme_t* fs, void(*Nonterminal)(void)) {
	syntax_errors = 0;
	lex = fs;
	Nonterminal();
	return lex->type == EOI && syntax_errors == 0;
}
void Program() {
	switch (lex->type) {
		case KW_PROGRAM:
			match(KW_PROGRAM);
			match(IDENTIFIER);
			match(STM_END);
			ConstantDeclarations();
			TypeDeclarations();
			FuncProcDeclarations();
			match(BLOCK_START);
			Grouping();
			match(BLOCK_END);
			match(PROGRAM_END);
			break;
		default:
			syntax_error(1, KW_PROGRAM);
	}
}
void ConstDeclPrime() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			match(OP_EQUALS);
			Expression();
			match(STM_END);
			ConstDeclPrime();
			break;
		default:
			return; //ConstDeclPrime -> epsilon
	}
}
void ConstDecl() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			match(OP_EQUALS);
			Expression();
			match(STM_END);
			ConstDeclPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
}
void ConstantDeclarations() {
	switch (lex->type) {
		case KW_CONST:
			match(KW_CONST);
			ConstDecl();
			break;
		default:
			return; //ConstantDeclarations->epsilon
	}
}
void Type() {
	switch (lex->type) {
		case TYPE_INTEGER:
			match(TYPE_INTEGER);
			break;
		case KW_ARRAY:
			match(KW_ARRAY);
			match(BRACKET_START);
			Expression();
			match(ARRAY_RANGE);
			Expression();
			match(BRACKET_END);
			match(KW_OF);
			match(TYPE_INTEGER);
			break;
		default:
			syntax_error(2, TYPE_INTEGER, KW_ARRAY);
	}
}
void VariablesPrime() {
	switch (lex->type) {
		case COMMA:
			match(COMMA);
			match(IDENTIFIER);
			VariablesPrime();
			break;
		default:
			return; //VariablesPrime -> epsilon
	}
}
void Variables() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			VariablesPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
}
void TypeDeclPrime() {
	switch (lex->type) {
		case IDENTIFIER:
			Variables();
			match(ASSIGN_TYPE);
			Type();
			match(STM_END);
			TypeDeclPrime();
			break;
		default:
			return; //TypeDeclPrime -> epsilon
	}
}
void TypeDecl() {
	switch (lex->type) {
		case IDENTIFIER:
			Variables();
			match(ASSIGN_TYPE);
			Type();
			match(STM_END);
			TypeDeclPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
}
void TypeDeclarations() {
	switch (lex->type) {
		case KW_VAR:
			match(KW_VAR);
			TypeDecl();
			TypeDeclarations();
			break;
		default:
			return; //TypeDeclarations -> epsilon
	}
}
void RestParams() {
	switch (lex->type) {
		case STM_END:
			match(STM_END);
			Variables();
			match(ASSIGN_TYPE);
			Type();
			RestParams();
			break;
		default:
			return; //RestParams -> epsilon
	}
}
void ParamsList() {
	switch (lex->type) {
		case PARENS_START:
			match(PARENS_START);
			Variables();
			match(ASSIGN_TYPE);
			Type();
			RestParams();
			match(PARENS_END);
			break;
		default:
			return; //ParamsList -> epsilon
	}
}
void ForwardOrCode() {
	switch (lex->type) {
		case KW_FORWARD:
			match(KW_FORWARD);
			match(STM_END);
			break;
		case KW_VAR:
		case BLOCK_START:
			TypeDeclarations();
			Block();
			match(STM_END);
			break;
		default:
			syntax_error(3, KW_FORWARD, KW_VAR, BLOCK_START);
			return;
	}
}
void ProcDecl() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			ParamsList();
			match(STM_END);
			ForwardOrCode();
			break;
		default:
			syntax_error(1, IDENTIFIER);
			return;
	}
}
void FuncDecl() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			ParamsList();
			match(ASSIGN_TYPE);
			Type();
			match(STM_END);
			ForwardOrCode();
			break;
		default:
			syntax_error(1, IDENTIFIER);
			return;
	}
}
void FuncProcDeclarations() {
	switch (lex->type) {
		case KW_PROCEDURE:
			match(KW_PROCEDURE);
			ProcDecl();
			FuncProcDeclarations();
			break;
		case KW_FUNCTION:
			match(KW_FUNCTION);
			FuncDecl();
			FuncProcDeclarations();
			break;
		default:
			return; //FuncProcDeclarations -> epsilon
	}
}
void Grouping() {
	switch (lex->type) {
		case KW_IF: //G -> if cond then Block G
			match(KW_IF);
			Expression();
			match(KW_THEN);
			Block();
			Else();
			Grouping();
			break;
		case KW_FOR:
			match(KW_FOR);
			match(IDENTIFIER);
			match(ASSIGN);
			Expression();
			ForDirection();
			Expression();
			match(KW_DO);
			Block();
			Grouping();
			break;
		case KW_WHILE:
			match(KW_WHILE);
			Expression();
			match(KW_DO);
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
void ForDirection() {
	switch (lex->type) {
		case KW_TO:
			match(KW_TO);
			break;
		case KW_DOWNTO:
			match(KW_DOWNTO);
			break;
		default:
			syntax_error(2, KW_TO, KW_DOWNTO);
	}
}
void ExprOrLiteral() {
	switch (lex->type) {
		case LITERAL:
			match(LITERAL);
			break;
		//First of expression
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			Expression();
			break;
		default:
			syntax_error(6, LITERAL, OP_NOT, OP_MINUS, NUMBER, IDENTIFIER, 
				PARENS_START);
			return;
	}
}
void RestArgs() {
	switch (lex->type) {
		case COMMA:
			match(COMMA);
			ExprOrLiteral();
			RestArgs();
			break;
		default: //RestArgs->epsilon
			return;
	}
}
void Arguments() {
	switch (lex->type) {
		//First of ExprOrLiteral
		case LITERAL:
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			ExprOrLiteral();
			RestArgs();
			break;
		default: //Arguments->epsilon
			return;
	}
}
void FuncCall() {
	switch (lex->type) {
		case PARENS_START:
			match(PARENS_START);
			Arguments();
			match(PARENS_END);
			break;
		default: //FuncCall -> $
			return;
	}
}
void IdentifierStatement() {
	switch (lex->type) {
		case ASSIGN: //Variable assignment
			match(ASSIGN);
			Expression();
			break;
		case BRACKET_START: //Array assignment
			match(BRACKET_START);
			Expression();
			match(BRACKET_END);
			match(ASSIGN);
			Expression();
			break;
		case PARENS_START: //Function call
			match(PARENS_START);
			Arguments();
			match(PARENS_END);
			break;
		default:
			syntax_error(3, ASSIGN, BRACKET_START, PARENS_START);
	}
}
void Statement() {
	switch (lex->type) {
		case IDENTIFIER:
			match(IDENTIFIER);
			IdentifierStatement();
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
			match(STM_END);
			break;
		case IDENTIFIER: //first of Statement
			Statement();
			match(STM_END);
			break;
		default:
			syntax_error(2, BLOCK_START, IDENTIFIER);
	}
}
void Expression() {
	switch (lex->type) {
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			Term();
			ExpressionPrime();
			break;
		default:
			syntax_error(5, NUMBER, IDENTIFIER, PARENS_START, OP_NOT, OP_MINUS);
	}
}
void ExpressionPrime() {
	switch (lex->type) {
		case OP_MOD:
			match(OP_MOD);
			Term();
			ExpressionPrime();
			break;
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
		case OP_MINUS:
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
		case OP_INTDIV:
			match(OP_INTDIV);
			Term();
			TermPrime();
			break;
		default:
			return;
		//No default error: TermPrime -> epsilon
	}
}
void FuncCallOrArrayIndex() {
	switch (lex->type) {
		case BRACKET_START:
			match(BRACKET_START);
			Expression();
			match(BRACKET_END);
			break;
		case PARENS_START:
			FuncCall();
		default:
			return;
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
			FuncCallOrArrayIndex();
			break;
		case OP_MINUS:
			match(OP_MINUS);
			Factor();
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
