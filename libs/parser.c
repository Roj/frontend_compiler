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
	fprintf(stderr, "Line %d: unexpected lexeme of type %s, expected ", 
		lex->line_num, lex2str(lex));
	for (int i = 0; i < num_options; i++) {
		//TODO: add a function that converts type to actual symbol
		//e.g. PARENS_START-> (
		fprintf(stderr, "%s", lextype2str(va_arg(arguments, lexeme_type_t)));
		if (i < num_options-2)
			fprintf(stderr, ", ");
		else if (i == num_options-2)
			fprintf(stderr, ", or ");
	}
	fprintf(stderr, " in non-terminal %s, at %s:%d.\n", func, file, line);
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
void store_identifier(char** dst) {
	*dst = malloc_assert(sizeof(char)*(strlen(lex->data.name)+1));
	strcpy(*dst, lex->data.name);
}
void store_number(int* dst) {
	*dst = lex->data.value; 
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
bool parse_unit_(lexeme_t* fs, void*(*Nonterminal)(void)) {
	syntax_errors = 0;
	lex = fs;
	Nonterminal();
	return lex->type == EOI && syntax_errors == 0;
}
NodeProgram* Program() {
	NodeProgram* prog = new_node(NodeProgram);
	switch (lex->type) {
		case KW_PROGRAM:
			match(KW_PROGRAM);
			store_identifier(&prog->name);
			match(IDENTIFIER);
			match(STM_END);
			prog->constdecl = ConstantDeclarations();
			prog->typedecl = TypeDeclarations();
			prog->fpdecl = FuncProcDeclarations();
			prog->typedecl2 = TypeDeclarations();
			match(BLOCK_START);
			prog->grouping = Grouping();
			match(BLOCK_END);
			match(PROGRAM_END);
			break;
		default:
			syntax_error(1, KW_PROGRAM);
	}
	return prog;
}
NodeConstDecl* ConstDeclPrime() {
	NodeConstDecl* node = NULL;
	switch (lex->type) {
		case IDENTIFIER:
			node = new_node(NodeConstDecl);
			match(IDENTIFIER);
			match(OP_EQUALS);
			Expression();
			match(STM_END);
			ConstDeclPrime();
			break;
		default:
			break; //ConstDeclPrime -> epsilon
	}
	return node;
}
NodeConstDecl* ConstDecl() {
	NodeConstDecl* constdecl = NULL;
	switch (lex->type) {
		case IDENTIFIER:
			constdecl = new_node(NodeConstDecl);
			store_identifier(&constdecl->name);
			match(IDENTIFIER);
			match(OP_EQUALS);
			constdecl->expression = Expression();
			match(STM_END);
			constdecl->next_constdecl = ConstDeclPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
	return constdecl;
}
NodeConstDecl* ConstantDeclarations() {
	switch (lex->type) {
		case KW_CONST:
			match(KW_CONST);
			return ConstDecl();
		default:
			return NULL; //ConstantDeclarations->epsilon
	}
}
NodeVariableType* Type() {
	NodeVariableType* node = new_node(NodeVariableType);
	switch (lex->type) {
		case TYPE_INTEGER:
			node->is_int = true;
			match(TYPE_INTEGER);
			break;
		case KW_ARRAY:
			node->is_int = false;
			match(KW_ARRAY);
			match(BRACKET_START);
			node->array_start = Expression();
			match(ARRAY_RANGE);
			node->array_end = Expression();
			match(BRACKET_END);
			match(KW_OF);
			match(TYPE_INTEGER);
			break;
		default:
			syntax_error(2, TYPE_INTEGER, KW_ARRAY);
	}
	return node;
}
NodeVariables* VariablesPrime() {
	NodeVariables* node = NULL;
	switch (lex->type) {
		case COMMA:
			node = new_node(NodeVariables);
			match(COMMA);
			store_identifier(&node->name);
			match(IDENTIFIER);
			node->next_variables = VariablesPrime();
			break;
		default:
			break; //VariablesPrime -> epsilon
	}
	return node;
}
NodeVariables* Variables() {
	NodeVariables* node = new_node(NodeVariables);
	switch (lex->type) {
		case IDENTIFIER:
			store_identifier(&node->name);
			match(IDENTIFIER);
			node->next_variables = VariablesPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
	return node;
}
NodeTypeDecl* TypeDeclPrime() {
	NodeTypeDecl* node = NULL;
	switch (lex->type) {
		case IDENTIFIER:
			node = new_node(NodeTypeDecl);
			node->variables = Variables();
			match(ASSIGN_TYPE);
			node->type = Type();
			match(STM_END);
			node->next_typedecl = TypeDeclPrime();
			break;
		default:
			break; //TypeDeclPrime -> epsilon
	}
	return node;
}
NodeTypeDecl* TypeDecl() {
	NodeTypeDecl* node = new_node(NodeTypeDecl);
	switch (lex->type) {
		case IDENTIFIER:
			node->variables = Variables();
			match(ASSIGN_TYPE);
			node->type = Type();
			match(STM_END);
			node->next_typedecl = TypeDeclPrime();
			break;
		default:
			syntax_error(1, IDENTIFIER);
	}
	return node;
}
NodeTypeDecl* TypeDeclarations() {
	NodeTypeDecl* node = NULL;
	switch (lex->type) {
		case KW_VAR:
			match(KW_VAR);
			node = TypeDecl();
			node->next_typedecl = TypeDeclarations();
			break;
		default:
			break; //TypeDeclarations -> epsilon
	}
	return node;
}
NodeParams* RestParams() {
	NodeParams* node = NULL;
	switch (lex->type) {
		case STM_END:
			node = new_node(NodeParams);
			match(STM_END);
			node->variables = Variables();
			match(ASSIGN_TYPE);
			node->type = Type();
			node->next_param = RestParams();
			break;
		default:
			break; //RestParams -> epsilon
	}
	return node;
}
NodeParams* ParamsList() {
	NodeParams* node = NULL;
	switch (lex->type) {
		case PARENS_START:
			node = new_node(NodeParams);
			match(PARENS_START);
			node->variables = Variables();
			match(ASSIGN_TYPE);
			node->type = Type();
			node->next_param = RestParams();
			match(PARENS_END);
			break;
		default:
			break; //ParamsList -> epsilon
	}
	return node;
}
void ForwardOrCode(bool* is_fwd, NodeTypeDecl** typedecl,
	NodeBlock** block) {
	switch (lex->type) {
		//TODO: we should only add to the symbol table here
		case KW_FORWARD:
			*is_fwd = true;
			match(KW_FORWARD);
			match(STM_END);
			break;
		case KW_VAR:
		case BLOCK_START:
			*is_fwd = false;
			*typedecl = TypeDeclarations();
			*block = Block();
			match(STM_END);
			break;
		default:
			syntax_error(3, KW_FORWARD, KW_VAR, BLOCK_START);
			return;
	}
}
NodeProcedure* ProcDecl() {
	NodeProcedure* node = new_node(NodeProcedure);
	switch (lex->type) {
		case IDENTIFIER:
			store_identifier(&node->name);
			match(IDENTIFIER);
			node->params = ParamsList();
			match(STM_END);
			ForwardOrCode(&node->is_forward, &node->typedecl, &node->block);
			break;
		default:
			syntax_error(1, IDENTIFIER);
			break;
	}
	return node;
}
NodeFunction* FuncDecl() {
	NodeFunction* node = new_node(NodeFunction);
	switch (lex->type) {
		case IDENTIFIER:
			store_identifier(&node->name);
			match(IDENTIFIER);
			node->params = ParamsList();
			match(ASSIGN_TYPE);
			Type(); //It should always be int.
			match(STM_END);
			ForwardOrCode(&node->is_forward, &node->typedecl, &node->block);
			break;
		default:
			syntax_error(1, IDENTIFIER);
			break;
	}
	return node;
}
NodeFPDecl* FuncProcDeclarations() {
	NodeFPDecl* node = NULL;
	switch (lex->type) {
		case KW_PROCEDURE:
			node = new_node(NodeFPDecl);
			match(KW_PROCEDURE);
			node->type = PROC;
			node->decl.proc = ProcDecl();
			node->next_fpdecl = FuncProcDeclarations();
			break;
		case KW_FUNCTION:
			node = new_node(NodeFPDecl);
			match(KW_FUNCTION);
			node->type = FUNC;
			node->decl.func = FuncDecl();
			node->next_fpdecl = FuncProcDeclarations();
			break;
		default:
			break; //FuncProcDeclarations -> epsilon
	}
	return node;
}
NodeGrouping* GroupingPrime() {
	switch (lex->type) {
		case STM_END:
			match(STM_END);
			return Grouping();
		default:
			return NULL; //GroupingPrime -> epsilon
	}
}
NodeGrouping* Grouping() {
	NodeGrouping* node = new_node(NodeGrouping);
	switch (lex->type) {
		case KW_IF: //G -> if cond then Block G
			node->type=IF;
			node->inner._if = new_node(NodeIf);
			match(KW_IF);
			node->inner._if->expr = Expression();
			match(KW_THEN);
			node->inner._if->block = Block();
			node->inner._if->_else = Else();
			node->next_grouping = GroupingPrime();
			break;
		case KW_FOR:
			node->type = FOR;
			node->inner._for = new_node(NodeFor);
			match(KW_FOR);
			store_identifier(&node->inner._for->id);
			match(IDENTIFIER);
			match(ASSIGN);
			node->inner._for->start_expr = Expression();
			node->inner._for->direction = ForDirection();
			node->inner._for->stop_expr = Expression();
			match(KW_DO);
			node->inner._for->block = Block();
			node->next_grouping = GroupingPrime();
			break;
		case KW_WHILE:
			node->type = WHILE;
			node->inner._while = new_node(NodeWhile);
			match(KW_WHILE);
			node->inner._while->condition = Expression();
			match(KW_DO);
			node->inner._while->block = Block();
			node->next_grouping = GroupingPrime();
			break;
		case KW_EXIT:
		case IDENTIFIER: //first of Statement
			node->type = STATEMENT;
			node->inner.statement = Statement();
			node->next_grouping = GroupingPrime();
			break;
		default:
			free(node);
			node = NULL;
			break;
	}
	return node;
}
fordirection_t ForDirection() {
	switch (lex->type) {
		case KW_TO:
			match(KW_TO);
			return UP;
			break;
		case KW_DOWNTO:
			match(KW_DOWNTO);
			return DOWN;
			break;
		default:
			syntax_error(2, KW_TO, KW_DOWNTO);
			return DOWN;
	}
}
void ExprOrLiteral(NodeArguments* node) {
	switch (lex->type) {
		case LITERAL:
			node->arg.literal = new_node(NodeLiteral);
			store_identifier(&node->arg.literal->value);
			match(LITERAL);
			break;
		//First of expression
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			node->is_expr = true;
			node->arg.expr = Expression();
			break;
		default:
			syntax_error(6, LITERAL, OP_NOT, OP_MINUS, NUMBER, IDENTIFIER, 
				PARENS_START);
			return;
	}
}
NodeArguments* RestArgs() {
	NodeArguments* node = NULL;
	switch (lex->type) {
		case COMMA:
			node = new_node(NodeArguments);
			match(COMMA);
			ExprOrLiteral(node);
			node->next_arg = RestArgs();
			break;
		default: //RestArgs->epsilon
			break;
	}
	return node;
}
NodeArguments* Arguments() {
	NodeArguments* node = NULL;
	switch (lex->type) {
		//First of ExprOrLiteral
		case LITERAL:
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			node = new_node(NodeArguments);
			ExprOrLiteral(node);
			node->next_arg = RestArgs();
			break;
		default: //Arguments->epsilon
			break;
	}
	return node;
}
NodeArguments* FuncCall() {
	NodeArguments* node = NULL;
	switch (lex->type) {
		case PARENS_START:
			match(PARENS_START);
			node = Arguments();
			match(PARENS_END);
			break;
		default: //FuncCall -> $
			break;
	}
	return node;
}
void IdentifierStatement(NodeStatement* node) {
	switch (lex->type) {
		case ASSIGN: //Variable assignment
			node->is_assign = true;
			match(ASSIGN);
			node->inner.assign = new_node(NodeAssign);
			node->inner.assign->expr = Expression();
			break;
		case BRACKET_START: //Array assignment
			node->is_assign = true;
			node->inner.assign = new_node(NodeAssign);
			match(BRACKET_START);
			node->inner.assign->arr_index = Expression();
			match(BRACKET_END);
			match(ASSIGN);
			node->inner.assign->expr = Expression();
			break;
		case PARENS_START: //Function call
			match(PARENS_START);
			node->inner.func_call_args = Arguments();
			match(PARENS_END);
			break;
		default:
			syntax_error(3, ASSIGN, BRACKET_START, PARENS_START);
	}
}
NodeStatement* Statement() {
	NodeStatement* node = new_node(NodeStatement);
	switch (lex->type) {
		case KW_EXIT:
			node->is_exit = true;
			match(KW_EXIT);
			break;
		case IDENTIFIER:
			store_identifier(&node->identifier);
			match(IDENTIFIER);
			IdentifierStatement(node);
			break;
		default:
			syntax_error(2, IDENTIFIER, KW_EXIT);
	}
	return node;
}
NodeElse* Else() {
	NodeElse* node = NULL;
	switch (lex->type) {
		case KW_ELSE:
			node = new_node(NodeElse);
			match(KW_ELSE);
			node->block = Block();
			break;
		default:
			break; //Else -> epsilon
	}
	return node;
}
NodeBlock* Block() {
	NodeBlock* node = new_node(NodeBlock);
	switch (lex->type) {
		case BLOCK_START:
			node->is_grouping = true;
			match(BLOCK_START);
			node->inner.grouping = Grouping();
			match(BLOCK_END);
			break;
		//First of Statement
		case KW_EXIT:
		case IDENTIFIER:
			node->inner.statement = Statement();
			break;
		default:
			syntax_error(3, BLOCK_START, IDENTIFIER, KW_EXIT);
	}
	return node;
}
NodeExpression* Expression() {
	NodeExpression* node = new_node(NodeExpression);
	switch (lex->type) {
		case OP_NOT:
		case OP_MINUS:
		case NUMBER:
		case IDENTIFIER:
		case PARENS_START:
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		default:
			syntax_error(5, NUMBER, IDENTIFIER, PARENS_START, OP_NOT, OP_MINUS);
	}
	return node;
}
NodeExpressionPrime* ExpressionPrime() {
	NodeExpressionPrime* node = new_node(NodeExpressionPrime);
	switch (lex->type) {
		case OP_MOD:
			node->op = MOD; 
			match(OP_MOD);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_GT:
			node->op = GT;
			match(OP_GT);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_GTE:
			node->op = GTE;
			match(OP_GTE);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_LT:
			node->op = LT;
			match(OP_LT);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_LTE:
			node->op = LTE;
			match(OP_LTE);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_NEQUALS:
			node->op = NEQ;
			match(OP_NEQUALS);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_EQUALS:
			node->op = EQ;
			match(OP_EQUALS);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_PLUS:
			node->op = PLUS;
			match(OP_PLUS);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_MINUS:
			node->op = MINUS;
			match(OP_MINUS);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_AND:
			node->op = AND;
			match(OP_AND);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		case OP_OR:
			node->op = OR;
			match(OP_OR);
			node->term = Term();
			node->exp_prime = ExpressionPrime();
			break;
		default:
			free(node);
			node = NULL;
			break;//ExpressionPrime -> epsilon
	}
	return node;
}
NodeTerm* Term() {
	NodeTerm* node = new_node(NodeTerm);
	switch (lex->type) {
		case OP_NOT:
		case OP_MINUS:
		case IDENTIFIER:
		case NUMBER:
		case PARENS_START:
			node->factor = Factor();
			node->term_prime = TermPrime();
			break;
		default:
			syntax_error(3, IDENTIFIER, NUMBER, PARENS_START);
	}
	return node;
}
NodeTermPrime* TermPrime() {
	NodeTermPrime* node = NULL;
	switch (lex->type) {
		case OP_MULTIPLY:
			node = new_node(NodeTermPrime);
			node->op = MULT;
			match(OP_MULTIPLY);
			node->factor = Factor();
			node->term_prime = TermPrime();
			break;
		case OP_DIV:
			node = new_node(NodeTermPrime);
			node->op = DIV;
			match(OP_DIV);
			node->factor = Factor();
			node->term_prime = TermPrime();
			break;
		case OP_INTDIV:
			node = new_node(NodeTermPrime);
			node->op = INTDIV;
			match(OP_INTDIV);
			node->factor = Factor();
			node->term_prime = TermPrime();
			break;
		default:
			break; //TermPrime -> epsilon
	}
	return node;
}
void FuncCallOrArrayIndex(NodeFactor* node) {
	switch (lex->type) {
		case BRACKET_START:
			node->type = ARRIDX;
			node->fac.array_index = new_node(NodeArrayIndex);
			match(BRACKET_START);
			node->fac.array_index->index = Expression();
			match(BRACKET_END);
			break;
		case PARENS_START:
			node->type = CALL;
			node->fac.call = new_node(NodeFuncCall);
			node->fac.call->args = FuncCall();
			break;
		default:
			node->type = IDENT;
			return;
	}
}
NodeFactor* Factor() {
	NodeFactor* node = new_node(NodeFactor);
	char* buf; 
	switch (lex->type) {
		case OP_NOT:
			node->type = NOT;
			match(OP_NOT);
			node->fac.inner_factor = Factor();
			break;
		case IDENTIFIER:
			buf = malloc_assert(sizeof(char)*(strlen(lex->data.name)+1));
			store_identifier(&buf);
			match(IDENTIFIER);
			FuncCallOrArrayIndex(node);
			if (node->type == CALL) {
				node->fac.call->fun_name = buf;
			} else if (node->type == IDENT) {
				node->fac.id = buf;
			} else if (node->type == ARRIDX){
				node->fac.array_index->name = buf;
			}
			break;
		case OP_MINUS:
			node->type = NEG;
			match(OP_MINUS);
			node->fac.inner_factor = Factor();
			break;
		case NUMBER:
			node->type = NUM;
			node->fac.num = new_node(NodeNumber);
			store_number(&node->fac.num->value);
			match(NUMBER);
			break;
		case PARENS_START:
			node->type = SUBEXPR;
			match(PARENS_START);
			node->fac.subexpr = Expression();
			match(PARENS_END);
			break;
		default:
			syntax_error(5, OP_NOT, IDENTIFIER, OP_MINUS, NUMBER, PARENS_START);
	}
	return node;
}
