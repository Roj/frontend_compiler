#ifndef _PARSER_H
#define _PARSER_H
#include "lexical.h"
#include "tree.h"

bool parse(lexeme_t* first_symbol);

bool parse_unit(lexeme_t* fs, void (*Nonterminal)(void));
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
