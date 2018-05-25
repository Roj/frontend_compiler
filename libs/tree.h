#ifndef _TREE_H
#define _TREE_H
#include "lexical.h"

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


typedef enum nodetype {
	NodeProgram,
	NodeConstantDeclaration,
	NodeTypeDeclaration,
	NodeVariableType,
	NodeVariables,
	NodeFPDeclaration,
	NodeFunction,
	NodeProcedure,
	NodeParams,
	NodeGrouping,
	NodeIf,
	NodeFor,
	NodeWhile,
	NodeBlock,
	NodeElse,
	NodeExpression,
	NodeStatement,
	NodeExitStatement,
	NodeAssign,
	NodeArguments,
	NodeTerm,
	NodeExpressionPrime,
	NodeTermPrime,
	NodeFactor,
	NodeFuncCall,
	//NodeExpressionOperator,
	//NodeFactorOperator,
	NodeIdentifier,
	NodeNumber,
	NodeLiteral
} node_type_t;

typedef struct node {
	node_type_t type;
	void* elem;
} node_t;

typedef struct NodeProgram node_program_t;
typedef struct NodeConstDecl node_constdecl_t;
typedef struct NodeTypeDecl node_typedecl_t;
typedef struct NodeVariableType node_variabletype_t;
typedef struct NodeVariables node_variables_t;
typedef struct NodeFPDecl node_fpdecl_t;
typedef struct NodeGrouping node_grouping_t;
typedef struct NodeFunction node_function_t;
typedef struct NodeProcedure node_procedure_t;
typedef struct NodeParams node_params_t;
typedef struct NodeIf node_if_t;
typedef struct NodeElse node_else_t;
typedef struct NodeFor node_for_t;
typedef struct NodeBlock node_block_t;
typedef struct NodeWhile node_while_t;
typedef struct NodeStatement node_statement_t;
typedef struct NodeAssign node_assign_t;
typedef struct NodeExpression node_expression_t;
typedef struct NodeExpressionPrime node_expression_prime_t;
typedef struct NodeTerm node_term_t;
typedef struct NodeTermPrime node_term_prime_t;
typedef struct NodeFactor node_factor_t;
typedef struct NodeArguments node_arguments_t;
typedef struct NodeNumber node_number_t;
typedef struct NodeLiteral node_literal_t;

struct NodeProgram {
	char* name;
	node_constdecl_t* constdecl;
	node_typedecl_t* typedecl;
	node_fpdecl_t* fpdecl;
	node_typedecl_t* typedecl2;
	node_grouping_t* grouping;
};

struct NodeConstDecl {
	char* name;
	node_expression_t* expression;
	node_constdecl_t* next_constdecl;
};

struct NodeTypeDecl {
	node_variables_t* variables;
	node_variabletype_t* type;
	node_typedecl_t* next_typedecl;
};

struct NodeVariables {
	char* name;
	node_variables_t* next_variables;
};

struct NodeFPDecl {
	//Of course this could be a void*, but explicit is 
	//better than implicit.
	union _inner_fpdecl {
		node_function_t* func;
		node_procedure_t* proc;
	} decl;
	node_fpdecl_t* next_fpdecl;
};

struct NodeGrouping {
	enum {IF, FOR, WHILE, STATEMENT} type;
	union _inner_grouping {
		node_if_t* _if;
		node_for_t* _for;
		node_while_t* _while;
		node_statement_t* statement;
	} inner;
	node_grouping_t* next_grouping;
};

struct NodeFunction {
	char* name;
	node_params_t* params;
	node_typedecl_t* typedecl;
	node_block_t* block;
};

struct NodeProcedure {
	char* name;
	node_params_t* params;
	node_typedecl_t* typedecl;
	node_block_t* block;
};

struct NodeParams {
	node_variables_t* variables;
	node_variabletype_t* type; //or assume int?
	node_params_t* nextparam;
};

struct NodeVariableType {
	bool is_int;
	int array_start;
	int array_end;
};

struct NodeIf {
	node_expression_t* expr;
	node_block_t* block;
	node_else_t* _else;
};

struct NodeElse {
	node_block_t* block;
};

struct NodeFor {
	char* id;
	node_expression_t* start_expr;
	fordirection_t direction;
	node_expression_t* stop_expr;
	node_block_t* block;
};

struct NodeBlock {
	bool is_grouping;
	union _inner_block {
		node_grouping_t* grouping;
		node_statement_t* statement;
	} inner;
};

struct NodeWhile {
	node_expression_t* condition;
	node_block_t* block;
};

struct NodeStatement {
	bool is_exit;
	bool is_assign;
	char* identifier;
	union _inner_statement {
		node_assign_t* assign;
		node_arguments_t* func_call_args;
	} type;
};

struct NodeAssign {
	node_expression_t* arr_index; //could be void (i.e. not array assign)
	node_expression_t* expr;
};

struct NodeExpression {
	node_term_t* term;
	node_expression_prime_t* exp_prime;
};

struct NodeExpressionPrime {
	expr_op_t op;
	node_term_t* term;
	node_expression_prime_t* exp_prime;
};

struct NodeTerm {
	node_factor_t* factor;
	node_term_prime_t* term_prime;
};

struct NodeTermPrime {
	factor_op_t op;
	node_factor_t* factor;
	node_term_prime_t* term_prime;
};

struct NodeFactor {
	enum {NEG, CALL, NUM, SUBEXPR} type;
	union _inner_factor {
		node_factor_t* negated_factor;
		node_arguments_t* arguments;
		node_number_t* num;
		node_expression_t* subexpr;
	} fac;
};

struct NodeArguments {
	bool is_expr;
	union _inner_arguments {
		node_expression_t* expr;
		node_literal_t* literal;
	} arg;
	node_arguments_t* nextarg;
};

struct NodeNumber {
	int value;
};
struct NodeLiteral {
	char* value;
};

#endif
