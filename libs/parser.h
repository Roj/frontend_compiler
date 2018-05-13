#ifndef _PARSER_H
#define _PARSER_H
#include "lexical.h"

bool parse(lexeme_t* first_symbol);

bool parse_unit(lexeme_t* fs, void (*Nonterminal)(void));
//This is not super-useful, but is necessary for unit testing.
void ConstantDeclarations();
void Grouping();
void Statement();
void Block();
void ForDirection();
void Else();
void Expression();
void ExpressionPrime();
void Term();
void TermPrime();
void Factor();


#endif
