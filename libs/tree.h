#ifndef _TREE_H
#define _TREE_H
#include "lexical.h"

#define new_node(x) (x*) malloc_assert(sizeof(x))

typedef enum expr_op {
	MOD,
	GTE,
	LTE,
	GT,
	LT,
	NEQ,
	EQ,
	PLUS,
	MINUS,
	AND,
	OR
} expr_op_t;

typedef enum factor_op {
	MULT,
	DIV,
	INTDIV
} factor_op_t;

typedef enum fordirection {
	UP,
	DOWN
} fordirection_t;

typedef struct NodeProgram NodeProgram;
typedef struct NodeConstDecl NodeConstDecl;
typedef struct NodeTypeDecl NodeTypeDecl;
typedef struct NodeVariableType NodeVariableType;
typedef struct NodeVariables NodeVariables;
typedef struct NodeFPDecl NodeFPDecl;
typedef struct NodeGrouping NodeGrouping;
typedef struct NodeFunction NodeFunction;
typedef struct NodeProcedure NodeProcedure;
typedef struct NodeParams NodeParams;
typedef struct NodeIf NodeIf;
typedef struct NodeElse NodeElse;
typedef struct NodeFor NodeFor;
typedef struct NodeBlock NodeBlock;
typedef struct NodeWhile NodeWhile;
typedef struct NodeStatement NodeStatement;
typedef struct NodeAssign NodeAssign;
typedef struct NodeExpression NodeExpression;
typedef struct NodeExpressionPrime NodeExpressionPrime;
typedef struct NodeTerm NodeTerm;
typedef struct NodeTermPrime NodeTermPrime;
typedef struct NodeFactor NodeFactor;
typedef struct NodeArguments NodeArguments;
typedef struct NodeNumber NodeNumber;
typedef struct NodeLiteral NodeLiteral;

struct NodeProgram {
	char* name;
	NodeConstDecl* constdecl;
	NodeTypeDecl* typedecl;
	NodeFPDecl* fpdecl;
	NodeTypeDecl* typedecl2;
	NodeGrouping* grouping;
};

struct NodeConstDecl {
	char* name;
	NodeExpression* expression;
	NodeConstDecl* next_constdecl;
};

struct NodeTypeDecl {
	NodeVariables* variables;
	NodeVariableType* type;
	NodeTypeDecl* next_typedecl;
};

struct NodeVariables {
	char* name;
	NodeVariables* next_variables;
};

struct NodeFPDecl {
	//Of course this could be a void*, but explicit is 
	//better than implicit.
	union _inner_fpdecl {
		NodeFunction* func;
		NodeProcedure* proc;
	} decl;
	NodeFPDecl* next_fpdecl;
};

struct NodeGrouping {
	enum {IF, FOR, WHILE, STATEMENT} type;
	union _inner_grouping {
		NodeIf* _if;
		NodeFor* _for;
		NodeWhile* _while;
		NodeStatement* statement;
	} inner;
	NodeGrouping* next_grouping;
};

struct NodeFunction {
	char* name;
	NodeParams* params;
	NodeTypeDecl* typedecl;
	NodeBlock* block;
};

struct NodeProcedure {
	char* name;
	NodeParams* params;
	NodeTypeDecl* typedecl;
	NodeBlock* block;
};

struct NodeParams {
	NodeVariables* variables;
	NodeVariableType* type; //or assume int?
	NodeParams* nextparam;
};

struct NodeVariableType {
	bool is_int;
	int array_start;
	int array_end;
};

struct NodeIf {
	NodeExpression* expr;
	NodeBlock* block;
	NodeElse* _else;
};

struct NodeElse {
	NodeBlock* block;
};

struct NodeFor {
	char* id;
	NodeExpression* start_expr;
	fordirection_t direction;
	NodeExpression* stop_expr;
	NodeBlock* block;
};

struct NodeBlock {
	bool is_grouping;
	union _inner_block {
		NodeGrouping* grouping;
		NodeStatement* statement;
	} inner;
};

struct NodeWhile {
	NodeExpression* condition;
	NodeBlock* block;
};

struct NodeStatement {
	bool is_exit;
	bool is_assign;
	char* identifier;
	union _inner_statement {
		NodeAssign* assign;
		NodeArguments* func_call_args;
	} type;
};

struct NodeAssign {
	NodeExpression* arr_index; //could be void (i.e. not array assign)
	NodeExpression* expr;
};

struct NodeExpression {
	NodeTerm* term;
	NodeExpressionPrime* exp_prime;
};

struct NodeExpressionPrime {
	expr_op_t op;
	NodeTerm* term;
	NodeExpressionPrime* exp_prime;
};

struct NodeTerm {
	NodeFactor* factor;
	NodeTermPrime* term_prime;
};

struct NodeTermPrime {
	factor_op_t op;
	NodeFactor* factor;
	NodeTermPrime* term_prime;
};

struct NodeFactor {
	enum {NEG, CALL, NUM, SUBEXPR} type;
	union _inner_factor {
		NodeFactor* negated_factor;
		NodeArguments* arguments;
		NodeNumber* num;
		NodeExpression* subexpr;
	} fac;
};

struct NodeArguments {
	bool is_expr;
	union _inner_arguments {
		NodeExpression* expr;
		NodeLiteral* literal;
	} arg;
	NodeArguments* nextarg;
};

struct NodeNumber {
	int value;
};
struct NodeLiteral {
	char* value;
};

void* malloc_assert(size_t size);
#endif
