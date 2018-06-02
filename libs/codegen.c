#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Object.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexical.h"
#include "parser.h"
#include "tree.h"

#include "../hashtable/table.h"

#define DUMP 1
#define TARGET_MACHINE_TRIPLE "x86_64-pc-linux-gnu" "x86_64-pc-linux-gnu" 

#define symbol_table_init(st) table_init((st), H_STRING, NULL, NULL)
#define symbol_exists(st, sn) table_contains((st), (hvalue_t) (sn))
#define symbol_get(st, sn) (LLVMValueRef) table_get((st), (hvalue_t) (sn))
#define symbol_add(st, sn, sv) table_set((st), (hvalue_t) (sn), (hvalue_t) (sv))

#define block_terminated(builder) (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock((builder))) !=  NULL)

typedef struct function_state {
	LLVMBuilderRef builder;
	LLVMValueRef ref;
	LLVMTypeRef type;
	char* name;
	table_t* symbols;
} function_state;
typedef struct program_state {
	LLVMModuleRef mod;
	table_t* symbols;
} program_state;

//Need forward declaration for these since they are used so much.
LLVMValueRef process_expression(NodeExpression* expr, function_state function,
	program_state program);
void process_grouping(NodeGrouping* grouping, function_state function,
	program_state program);

void gen_print_number(LLVMBuilderRef builder, LLVMValueRef var,
	table_t* global_symbols) {
	if (! symbol_exists(global_symbols, "_printf_format"))
		symbol_add(global_symbols, "_printf_format",
			LLVMBuildGlobalStringPtr(builder, "%d.\n", "_printf_format"));

	LLVMValueRef format = symbol_get(global_symbols, "_printf_format");
	LLVMValueRef PrintfArgs[] = {format, var};

	LLVMBuildCall(builder, symbol_get(global_symbols, "printf"),
		PrintfArgs, 2, "printf");
}
void read_number(LLVMBuilderRef builder, LLVMValueRef ptr,
	table_t* global_symbols) {
	if (! symbol_exists(global_symbols, "_scanf_format"))
		symbol_add(global_symbols, "_scanf_format",
			LLVMBuildGlobalStringPtr(builder, "%d", "_scanf_format"));

	LLVMValueRef format = symbol_get(global_symbols, "_scanf_format");
	LLVMValueRef ScanfArgs[] = {format, ptr};

	LLVMBuildCall(builder, symbol_get(global_symbols, "scanf"),
		ScanfArgs, 2, "scanf");

}
LLVMValueRef get_closest_symbol(char* name, function_state function,
	program_state program) {
	if (symbol_exists(function.symbols, name))
		return symbol_get(function.symbols, name);

	if (symbol_exists(program.symbols, name))
		return symbol_get(program.symbols, name);

	return NULL;
}
LLVMValueRef handle_factor_op(LLVMBuilderRef builder,
	LLVMValueRef v1, LLVMValueRef v2, factor_op_t op) {
	switch (op) {
		case MULT:
			return LLVMBuildMul(builder, v1, v2, "multresult");
		case DIV:
			return LLVMBuildSDiv(builder, v1, v2, "divresult");
		case INTDIV:
			//UHHH idk the difference
			return LLVMBuildSDiv(builder, v1, v2, "divresult");
	}
	return NULL;
}
LLVMValueRef handle_expr_op(LLVMBuilderRef builder, 
	LLVMValueRef v1, LLVMValueRef v2, expr_op_t op) {
	switch (op) {
		case MOD:
			return LLVMBuildSRem(builder, v1, v2, "modresult");
		case GTE:
			return LLVMBuildICmp(builder, LLVMIntSGE, v1, v2, "gteresult");
		case LTE:
			return LLVMBuildICmp(builder, LLVMIntSLE, v1, v2, "lteresult");
		case GT:
			return LLVMBuildICmp(builder, LLVMIntSGT, v1, v2, "gtresult");
		case LT:
			return LLVMBuildICmp(builder, LLVMIntSLT, v1, v2, "ltresult");
		case NEQ:
			return LLVMBuildICmp(builder, LLVMIntNE, v1, v2, "neqresult");
		case EQ:
			return LLVMBuildICmp(builder, LLVMIntEQ, v1, v2, "eqresult");
		case PLUS:
			return LLVMBuildAdd(builder, v1, v2, "addresult");
		case MINUS:
			return LLVMBuildSub(builder, v1, v2, "subresult");
		case AND:
			return LLVMBuildAnd(builder, v1, v2, "andresult");
		case OR:
			return LLVMBuildOr(builder, v1, v2, "orresult");
	}
	return NULL;
}
LLVMValueRef process_funccall(char* fname, NodeArguments* node_args, 
	function_state function, program_state program) {
	LLVMValueRef func = get_closest_symbol(fname, function, program);
	if (func == NULL) {
		fprintf(stderr, "function symbol not found: %s\n", fname);
		return NULL;
	}
	//Handle recursion case.
	if (strcmp(fname, function.name) == 0)
		func = function.ref;

	int num_args_passed = get_num_arguments(node_args);
	LLVMValueRef* args = malloc(sizeof(LLVMValueRef) * num_args_passed);
	for (int i = 0; i < num_args_passed; i++) {
		if (! node_args->is_expr) { //TODO
			fprintf(stderr, "passing value is not expression\n");
			return NULL;
		}
		args[i] = process_expression(node_args->arg.expr, function, program);
		node_args = node_args->next_arg;
	}
	return LLVMBuildCall(function.builder, func, args, num_args_passed, "");
}
LLVMValueRef process_factor(NodeFactor* factor, function_state function,
	program_state program) {
	switch (factor->type) {
		case NOT:
			return LLVMBuildNot(
				function.builder,
				process_factor(factor->fac.inner_factor, function, program),
				"negatedfactor"
			);
		case NEG:
			return LLVMBuildNeg(
				function.builder,
				process_factor(factor->fac.inner_factor, function, program),
				"minusfactor"
			);
		case CALL:
			return process_funccall(factor->fac.call->fun_name,
				factor->fac.call->args, function, program);
		case IDENT:
			;
			LLVMValueRef val = get_closest_symbol(factor->fac.id, function, program);

			if (val == NULL) {
				fprintf(stderr, "identifier %s not found\n", factor->fac.id);
				return NULL;
			}
			if (LLVMIsConstant(val))
				return val;

			return LLVMBuildLoad(function.builder, val, "load_");
		case ARRIDX:
			fprintf(stderr, "arridx not implemented as factor\n");
			break;
		case NUM:
			return LLVMConstInt(LLVMInt32Type(), factor->fac.num->value, 0);
		case SUBEXPR:
			return process_expression(factor->fac.subexpr, function, program);
	}
	return NULL;
}
LLVMValueRef process_term_prime(NodeTermPrime* termprime,
	function_state function, program_state program) {
	if (termprime == NULL)
		return NULL;
	LLVMValueRef result = process_factor(termprime->factor, function, program);
	if (termprime->term_prime != NULL)
		result = handle_factor_op(
			function.builder,
			result,
			process_term_prime(termprime->term_prime, function, program),
			termprime->term_prime->op
		);
	return result;
}
LLVMValueRef process_term(NodeTerm* term, function_state function, 
	program_state program) {
	LLVMValueRef result = process_factor(term->factor, function, program);
	if (term->term_prime != NULL)
		result = handle_factor_op(
			function.builder,
			result,
			process_term_prime(term->term_prime, function, program),
			term->term_prime->op
		);
	return result;
}
LLVMValueRef process_expressionprime(NodeExpressionPrime* expr_prime,
	function_state function, program_state program) {
	if (expr_prime == NULL) 
		return NULL;
	
	LLVMValueRef expr_prime_result = process_expressionprime(
		expr_prime->exp_prime,
		function,
		program
	);
	
	LLVMValueRef expr_result = process_term(expr_prime->term, function, program);
	if (expr_prime_result != NULL) 
		expr_result = handle_expr_op(
			function.builder,
			expr_result,
			expr_prime_result,
			expr_prime->exp_prime->op
		);
	return expr_result;
}
LLVMValueRef process_expression(NodeExpression* expr, function_state function,
	program_state program) {
	LLVMValueRef expr_prime_result = process_expressionprime(
		expr->exp_prime,
		function,
		program
	);
	LLVMValueRef expr_result = process_term(expr->term, function, program);
	if (expr_prime_result != NULL) 
		expr_result = handle_expr_op(
			function.builder,
			expr_result,
			expr_prime_result,
			expr->exp_prime->op
		);
	return expr_result;
}
void process_statement(NodeStatement* statement, function_state function,
	program_state program) {
	if (block_terminated(function.builder))
		return;
	if (statement->is_assign) {
		LLVMValueRef variable = get_closest_symbol(statement->identifier,
			function, program);

		if (variable == NULL) {
			return;
		}

		LLVMBuildStore(
			function.builder,
			process_expression(statement->inner.assign->expr, function, program),
			variable
		);
		return;
	} 
	if (statement->is_exit) {
		if (strcmp(function.name, "main") == 0) {
			//Always return 0.
			LLVMBuildRet(function.builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
			return;
		}
		if (LLVMGetReturnType(function.type) == LLVMVoidType()) {
			LLVMBuildRetVoid(function.builder);
			return;
		}
		LLVMBuildRet(function.builder, 
			LLVMBuildLoad(function.builder, 
				symbol_get(function.symbols, function.name), ""));
		return;
	}
	//Function (or procedure) call
	NodeArguments* args = statement->inner.func_call_args;
	if (strcmp(statement->identifier, "writeln") == 0)  {
		gen_print_number(
			function.builder, 
			process_expression(args->arg.expr, function, program),
			program.symbols
		);
	} else if (strcmp(statement->identifier, "readln") == 0) {
		if (get_num_arguments(statement->inner.func_call_args) != 1) {
			fprintf(stderr, "Readln call with wrong num of args\n");
			return;
		}
		char* id = get_name_if_arg_is_identifier(statement->inner.func_call_args);
		if (id == NULL) {
			fprintf(stderr, "Readln was not passed a pointer\n");
			return;
		}
		LLVMValueRef ptr = get_closest_symbol(id, function, program);
		if (ptr == NULL) {
			fprintf(stderr, "Symbol not found\n");
			return;
		}
		read_number(
			function.builder,
			ptr,
			program.symbols
		);
	} else if (strcmp(statement->identifier, "dec") == 0
		|| strcmp(statement->identifier, "inc") == 0) {
		if (get_num_arguments(statement->inner.func_call_args) != 1) {
			fprintf(stderr, "Inc/dec call with wrong num of args\n");
			return;
		}
		char* id = get_name_if_arg_is_identifier(statement->inner.func_call_args);
		if (id == NULL) {
			fprintf(stderr, "Inc/dec was not passed a pointer\n");
			return;
		}
		LLVMValueRef ptr = get_closest_symbol(id, function, program);
		if (ptr == NULL) {
			fprintf(stderr, "Symbol not found\n");
			return;
		}
		int direction = (strcmp(statement->identifier, "inc") == 0)? 1:-1;
		LLVMBuildStore(
			function.builder,
			LLVMBuildAdd(function.builder,
				LLVMBuildLoad(function.builder, ptr, "incdec_result"),
				LLVMConstInt(LLVMInt32Type(), direction, 0), "incdirection"),
			ptr
		);
	} else {
		process_funccall(statement->identifier, statement->inner.func_call_args,
			function, program);
	}
}
void process_block(NodeBlock* block, function_state function,
	program_state program) {
	if (block == NULL)
		return;

	if (block->is_grouping) {
		process_grouping(block->inner.grouping, function, program);
	} else {
		process_statement(block->inner.statement, function, program);
	}
}
void process_if(NodeIf* if_node, function_state function, program_state program) {
	/*
	 * Basically the flow we create is:
	 main_block:
	 branch <expr>, then_block
	 jump else_block
	 then_block:
	 ...
	 jump continue_block
	 else_block:
	 [...] //optional, may be empty
	 jump continue_block
	 continue_block:
	 */
	LLVMBasicBlockRef then_blockref = LLVMAppendBasicBlock(
		function.ref, "then_block");
	LLVMBasicBlockRef else_blockref = LLVMAppendBasicBlock(
		function.ref, "else_block");
	LLVMBasicBlockRef continue_blockref = LLVMAppendBasicBlock(
		function.ref, "continue_block");

	LLVMBuildCondBr(
		function.builder,
		process_expression(if_node->expr, function, program),
		then_blockref,
		else_blockref
	);

	LLVMPositionBuilderAtEnd(function.builder, then_blockref);
	process_block(if_node->block, function, program);
	if (! block_terminated(function.builder))
		LLVMBuildBr(function.builder, continue_blockref);

	LLVMPositionBuilderAtEnd(function.builder, else_blockref);
	if (if_node->_else != NULL)
		process_block(if_node->_else->block, function, program);

	if (! block_terminated(function.builder))
		LLVMBuildBr(function.builder, continue_blockref);

	LLVMPositionBuilderAtEnd(function.builder, continue_blockref);
}
void process_for(NodeFor* _for, function_state function, program_state program) {
	/*
	 * Flow idea:
	 main_block:
	 i = <start_expr>
	 jump for_condition
	 for_condition:
	 branch <expr>, for_block
	 jump outside_for
	 for_block:
	 [...]
	 i = +|- 1
	 jump for_condition
	 outside_for:
	 */
	LLVMBasicBlockRef condition_blockref = LLVMAppendBasicBlock(
		function.ref, "for_condition");
	LLVMBasicBlockRef for_blockref = LLVMAppendBasicBlock(
		function.ref, "for_block");
	LLVMBasicBlockRef outside_blockref = LLVMAppendBasicBlock(
		function.ref, "outside_for");

	LLVMValueRef it_var = get_closest_symbol(_for->id, function, program);
	if (it_var == NULL) {
		fprintf(stderr, "Symbol %s not found\n", _for->id);
		return;
	}
	LLVMBuildStore(
		function.builder,
		process_expression(_for->start_expr, function, program),
		it_var
	);
	LLVMBuildBr(function.builder, condition_blockref);
	LLVMPositionBuilderAtEnd(function.builder, condition_blockref);

	LLVMValueRef cond = LLVMBuildICmp(
		function.builder,
		(_for->direction == UP)? LLVMIntSLE : LLVMIntSGE,
		LLVMBuildLoad(function.builder, it_var, "load_itvar"),
		process_expression(_for->stop_expr, function, program),
		"forcond"
	);

	LLVMBuildCondBr(
		function.builder,
		cond,
		for_blockref,
		outside_blockref
	);

	LLVMPositionBuilderAtEnd(function.builder, for_blockref);
	process_block(_for->block, function, program);
	if (! block_terminated(function.builder)) {
		LLVMValueRef new_val_it;
		if (_for->direction == UP)
			new_val_it = LLVMBuildAdd(function.builder,
				LLVMBuildLoad(function.builder, it_var, "load_itvar"),
				LLVMConstInt(LLVMInt32Type(), 1, 0), "add_it_var");
		else
			new_val_it = LLVMBuildSub(function.builder,
				LLVMBuildLoad(function.builder, it_var, "load_itvar"),
				LLVMConstInt(LLVMInt32Type(), 1, 0), "sub_it_var");

		LLVMBuildStore(
			function.builder,
			new_val_it,
			it_var
		);
		LLVMBuildBr(function.builder, condition_blockref);
	}
	LLVMPositionBuilderAtEnd(function.builder, outside_blockref);
}
void process_while(NodeWhile* _while, function_state function,
	program_state program) {
	/*
	 * Flow idea:
	 main_block:
	 jump while_condition
	 while_condition:
	 branch <expr>, while_block
	 jump outside_while
	 while_block:
	 [...]
	 jump while_condition
	 outside_while:
	 */
	LLVMBasicBlockRef condition_blockref = LLVMAppendBasicBlock(
		function.ref, "while_condition");
	LLVMBasicBlockRef while_blockref = LLVMAppendBasicBlock(
		function.ref, "while_block");
	LLVMBasicBlockRef outside_blockref = LLVMAppendBasicBlock(
		function.ref, "outside_while");

	LLVMBuildBr(function.builder, condition_blockref);
	LLVMPositionBuilderAtEnd(function.builder, condition_blockref);

	LLVMBuildCondBr(
		function.builder,
		process_expression(_while->condition, function, program),
		while_blockref,
		outside_blockref
	);

	LLVMPositionBuilderAtEnd(function.builder, while_blockref);
	process_block(_while->block, function, program);
	if (! block_terminated(function.builder))
		LLVMBuildBr(function.builder, condition_blockref);

	LLVMPositionBuilderAtEnd(function.builder, outside_blockref);
}
void process_grouping(NodeGrouping* grouping, function_state function,
	program_state program) {
	if (grouping == NULL || block_terminated(function.builder))
		return;
	switch (grouping->type) {
		case STATEMENT:
			process_statement(grouping->inner.statement, function, program);
			break;
		case IF:
			process_if(grouping->inner._if, function, program);
			break;
		case FOR:
			process_for(grouping->inner._for, function, program);
			break;
		case WHILE:
			process_while(grouping->inner._while, function, program);
			break;
	}

	process_grouping(grouping->next_grouping, function, program);
}
//Since this function can be called for the global scope ("main function") 
//or for a specific function, we need to be able to tell which symbol table
//we should add to.
void process_constdecl(NodeConstDecl* decl, function_state function,
	program_state program, table_t* symbols) {
	if (decl == NULL)
		return;

	symbol_add(symbols, decl->name,
		process_expression(decl->expression, function, program));

	process_constdecl(decl->next_constdecl, function, program, symbols);
}
void process_typedecl(NodeTypeDecl* typedecl, function_state function,
	program_state program, table_t* symbols) {
	if (typedecl == NULL)
		return;
	//TODO: add array support?
	//TODO: should I copy the str?
	NodeVariables* variable = typedecl->variables;
	for (; variable != NULL; variable = variable->next_variables) {
		symbol_add(
			symbols,
			variable->name,
			LLVMBuildAlloca(function.builder, LLVMInt32Type(), variable->name)
		);
	}
	process_typedecl(typedecl->next_typedecl, function, program, symbols);
}
void process_parameters(NodeParams* params, function_state function) {
	int i = 0;
	while (params) {
		//TODO: check type to differentiate b/w array and int.
		NodeVariables* variable = params->variables;
		while (variable) {
			LLVMValueRef passed_value = LLVMGetParam(function.ref, i);
			LLVMValueRef ptr = LLVMBuildAlloca(function.builder, 
				LLVMInt32Type(), variable->name);
			LLVMBuildStore(
				function.builder,
				passed_value,
				ptr
			);
			symbol_add(
				function.symbols,
				variable->name,
				ptr
			);
			variable = variable->next_variables;
			i++;
		}
		params = params->next_param;
	}
}
void process_funcdecl(NodeFunction* fdecl, program_state global_state) {
	LLVMValueRef fref = NULL;
	if (! symbol_exists(global_state.symbols, fdecl->name)) {
		int num_params = get_num_params(fdecl->params);
		//TODO: does llvm keep the memory or copy it? free in second case.
		LLVMTypeRef* params = malloc(sizeof(LLVMTypeRef) * num_params);
		for(int i = 0; i < num_params; i++)
			params[i] = LLVMInt32Type();
		LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), params,
			num_params, false);
		LLVMValueRef fref = LLVMAddFunction(global_state.mod, fdecl->name, ret_type);
		symbol_add(global_state.symbols, fdecl->name, fref);
	}
	//We put in here in case there are duplicated forwards. Shouldn't cause any
	//errors.
	if (fdecl->is_forward)
		return;

	fref = symbol_get(global_state.symbols, fdecl->name);

	LLVMBasicBlockRef block = LLVMAppendBasicBlock(fref, "_functionstart");
	LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, block);

	table_t* function_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(function_symbols);

	function_state fstate = {builder, fref, LLVMTypeOf(fref), 
		fdecl->name, function_symbols};

	//In Pascal, the function name is a variable that can be used as a return
	//value holder.
	LLVMValueRef retholder = LLVMBuildAlloca(builder, LLVMInt32Type(), 
		fdecl->name);
	symbol_add(function_symbols, fdecl->name, retholder);
	//Process code, like in the main function.
	process_parameters(fdecl->params, fstate);
	process_typedecl(fdecl->typedecl, fstate, global_state, function_symbols);
	process_block(fdecl->block, fstate, global_state);

	LLVMBuildRet(builder, LLVMBuildLoad(builder, retholder, "load_retvalue"));

}
void process_procdecl(NodeProcedure* pdecl, program_state global_state) {
	LLVMValueRef pref = NULL;
	if (! symbol_exists(global_state.symbols, pdecl->name)) {
		int num_params = get_num_params(pdecl->params);
		//TODO: does llvm keep the memory or copy it? free in second case.
		LLVMTypeRef* params = malloc(sizeof(LLVMTypeRef) * num_params);
		for(int i = 0; i < num_params; i++)
			params[i] = LLVMInt32Type();
		LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(), params,
			num_params, false);
		pref = LLVMAddFunction(global_state.mod, pdecl->name, ret_type);
	}
	//We put in here in case there are duplicated forwards. Shouldn't cause any
	//errors.
	if (pdecl->is_forward)
		return;

	pref = symbol_get(global_state.symbols, pdecl->name);

	LLVMBasicBlockRef block = LLVMAppendBasicBlock(pref, "_procstart");
	LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, block);

	table_t* function_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(function_symbols);

	function_state fstate = {builder, pref, LLVMTypeOf(pref),
		pdecl->name, function_symbols};

	//Process code, like in the main function.
	process_parameters(pdecl->params, fstate);
	process_typedecl(pdecl->typedecl, fstate, global_state, function_symbols);
	process_block(pdecl->block, fstate, global_state);

	LLVMBuildRetVoid(builder);

	symbol_add(global_state.symbols, pdecl->name, pref);
}
void process_fpdecl(NodeFPDecl* fpdecl, program_state global_state) {
	if (fpdecl == NULL)
		return;

	if (fpdecl->type == FUNC)
		process_funcdecl(fpdecl->decl.func, global_state);
	else
		process_procdecl(fpdecl->decl.proc, global_state);

	process_fpdecl(fpdecl->next_fpdecl, global_state);
}
void process_ast(NodeProgram* root, LLVMModuleRef mod) {
	//main function declaration
	LLVMTypeRef main_ret = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
	LLVMValueRef mainf = LLVMAddFunction(mod, "main", main_ret);

	//Add extern reference to function print
	LLVMTypeRef param_printf[] = { LLVMPointerType(LLVMInt8Type(), 0) };
	LLVMTypeRef ret_type_print = LLVMFunctionType(LLVMInt32Type(), param_printf, 0, true);
	LLVMValueRef printf_ = LLVMAddFunction(mod, "printf", ret_type_print);

	//Add extern reference to function scanf
	LLVMTypeRef param_scanf[] = { LLVMPointerType(LLVMInt8Type(), 0) };
	LLVMTypeRef ret_type_scan = LLVMFunctionType(LLVMInt32Type(), param_scanf, 0, true);
	LLVMValueRef scanf_ = LLVMAddFunction(mod, "scanf", ret_type_scan);

	//main() code
	LLVMBasicBlockRef main_block = LLVMAppendBasicBlock(mainf, "_start");
	LLVMBuilderRef main_builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(main_builder, main_block);

	table_t* global_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(global_symbols);
	program_state global_state = {mod, global_symbols};


	table_t* function_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(function_symbols);
	function_state main_state = {main_builder, mainf, main_ret, 
		"main", function_symbols};

	//load global symbols
	symbol_add(global_symbols, "printf", printf_);
	symbol_add(global_symbols, "scanf", scanf_);
	process_constdecl(root->constdecl, main_state, global_state, global_symbols);
	//load functions
	process_fpdecl(root->fpdecl, global_state);
	//main block code
	process_typedecl(root->typedecl, main_state, global_state, global_symbols);
	process_grouping(root->grouping, main_state, global_state);

	table_destroy(main_state.symbols);
	free(main_state.symbols);

	table_destroy(global_state.symbols);
	free(global_state.symbols);

	//Always return 0.
	LLVMBuildRet(main_builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
	LLVMDisposeBuilder(main_builder);
}
//Entry point for code generation. Sets up the module, calls to process_ast
//and writes an object file for a hardcoded target.
void codegen(NodeProgram* root, char* fileout) {
	LLVMModuleRef mod = LLVMModuleCreateWithName(root->name);

	process_ast(root, mod);
	
	char* error = NULL;
	LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
	LLVMDisposeMessage(error);

	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();
	LLVMInitializeNativeAsmParser();
	
	if (DUMP)
		LLVMDumpModule(mod);
	
	char* msg = NULL;
	
	//Try to output an object file
	const char* target_triple = TARGET_MACHINE_TRIPLE;
	LLVMSetTarget(mod, target_triple);
	LLVMTargetRef targetref;
	if (LLVMGetTargetFromTriple(target_triple, &targetref, &msg) != 0) {
		printf("Error getting Target: %s\n", msg);
		LLVMDisposeMessage(msg);
	}

	LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
		targetref,
		target_triple,
		"broadwell",
		"",
		LLVMCodeGenLevelDefault,
		LLVMRelocDefault,
		LLVMCodeModelDefault);
	LLVMBool emit_result = LLVMTargetMachineEmitToFile(
		target_machine,
		mod,
		fileout,
		LLVMObjectFile,
		&msg);
	if (emit_result != 0) {
		printf("Error while emitting .o file: %s", msg);
		LLVMDisposeMessage(msg);
	} else {
		printf("Saved object data to %s\n", fileout);
	}
	LLVMDisposeTargetMachine(target_machine);
}
