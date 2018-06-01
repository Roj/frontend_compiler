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

typedef struct function_state {
	LLVMBuilderRef builder;
	LLVMValueRef function_ref;
	table_t* symbols;
} function_state;
typedef struct program_state {
	LLVMModuleRef mod;
	LLVMValueRef printf_ref;
	table_t* symbols;
} program_state;

//Need forward declaration for these since they are used so much.
LLVMValueRef process_expression(NodeExpression* expr, function_state function,
	program_state program);
void process_grouping(NodeGrouping* grouping, function_state function,
	program_state program);

void gen_print_number(LLVMBuilderRef builder, LLVMValueRef var, 
	LLVMValueRef printf_ref) {
	LLVMValueRef format = LLVMBuildGlobalStringPtr(
		builder,
		"%d.\n",
		"format"
	);
	//We use the same identifiers for each function call, but LLVM automatically
	//appends a number to differentiate between calls
	LLVMValueRef PrintfArgs[] = { format, var};
	LLVMBuildCall(builder, printf_ref, PrintfArgs, 2, "printf");
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
			//TODO
			break;
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
			fprintf(stderr, "call not implemented as factor\n");
			break;
		case IDENT:
			;
			LLVMValueRef val = NULL;
			if (symbol_exists(function.symbols, factor->fac.id))
				val = symbol_get(function.symbols, factor->fac.id);
			
			if (val == NULL && symbol_exists(program.symbols, factor->fac.id))
				val = symbol_get(program.symbols, factor->fac.id);

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
	if (statement->is_assign) {
		LLVMValueRef variable = NULL;
		variable = symbol_get(function.symbols, statement->identifier);
		if (variable == NULL)
			variable = symbol_get(program.symbols, statement->identifier);

		if (variable == NULL) {
			fprintf(stderr, "Symbol not recognised: %s\n", statement->identifier);
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
		return;
	}
	//Function call
	NodeArguments* args = statement->inner.func_call_args;
	if (strcmp(statement->identifier, "writeln") == 0)  {
		gen_print_number(
			function.builder, 
			process_expression(args->arg.expr, function, program),
			program.printf_ref
		);
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
void process_grouping(NodeGrouping* grouping, function_state function,
	program_state program) {
	if (grouping == NULL)
		return;
	switch (grouping->type) {
		case STATEMENT:
			process_statement(grouping->inner.statement, function, program);
			break;
		case IF:
			;
			NodeIf* if_node = grouping->inner._if;

			LLVMBasicBlockRef then_blockref = LLVMAppendBasicBlock(
				function.function_ref, "then_block");
			LLVMBasicBlockRef else_blockref = LLVMAppendBasicBlock(
				function.function_ref, "else_block");
			LLVMBasicBlockRef continue_blockref = LLVMAppendBasicBlock(
				function.function_ref, "continue_block");

			LLVMBuildCondBr(
				function.builder,
				process_expression(if_node->expr, function, program),
				then_blockref,
				else_blockref
			);

			LLVMPositionBuilderAtEnd(function.builder, then_blockref);
			process_block(if_node->block, function, program);
			LLVMBuildBr(function.builder, continue_blockref);

			LLVMPositionBuilderAtEnd(function.builder, else_blockref);
			if (if_node->_else != NULL)
				process_block(if_node->_else->block, function, program);

			LLVMBuildBr(function.builder, continue_blockref);

			LLVMPositionBuilderAtEnd(function.builder, continue_blockref);
			break;
		case FOR:
			break;
		case WHILE:
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
}
void process_ast(NodeProgram* root, LLVMModuleRef mod) {
	//main function declaration
	LLVMTypeRef main_ret = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
	LLVMValueRef mainf = LLVMAddFunction(mod, "main", main_ret);

	//Add extern reference to function print
	LLVMTypeRef param_printf[] = { LLVMPointerType(LLVMInt8Type(), 0) };
	LLVMTypeRef ret_type_print = LLVMFunctionType(LLVMInt32Type(), param_printf, 0, true);
	LLVMValueRef printf_ = LLVMAddFunction(mod, "printf", ret_type_print);

	//main() code
	LLVMBasicBlockRef main_block = LLVMAppendBasicBlock(mainf, "_start");
	LLVMBuilderRef main_builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(main_builder, main_block);

	table_t* global_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(global_symbols);
	program_state global_state = {mod, printf_, global_symbols};

	table_t* function_symbols = malloc_assert(sizeof(table_t));
	symbol_table_init(function_symbols);
	function_state main_state = {main_builder, mainf, function_symbols};

	//load global symbols
	process_constdecl(root->constdecl, main_state, global_state, global_symbols);
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
