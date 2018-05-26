#ifndef _PARSER_H
#define _PARSER_H
#include "lexical.h"
#include "tree.h"

typedef void* (*Nonterminal)(void);

bool parse(lexeme_t* first_symbol);

#define parse_unit(i, f) parse_unit_((i),(Nonterminal) (f))
bool parse_unit_(lexeme_t* fs, void* (*Nonterminal)(void));

//This is not super-useful, but is necessary for unit testing.

NodeProgram* Program();
NodeConstDecl* ConstantDeclarations();
NodeTypeDecl* TypeDeclarations();
NodeFPDecl* FuncProcDeclarations();
NodeGrouping* Grouping();
NodeStatement* Statement();
NodeBlock* Block();
fordirection_t ForDirection();
NodeElse* Else();
NodeExpression* Expression();
NodeExpressionPrime* ExpressionPrime();
NodeTerm* Term();
NodeTermPrime* TermPrime();
NodeFactor* Factor();


#endif
