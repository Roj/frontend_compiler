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

typedef struct function_state {
	LLVMBuilderRef builder;
	table_t* symbols;
} function_state;
typedef struct program_state {
	LLVMModuleRef mod;
	LLVMValueRef printf_ref;
	table_t* symbols;
} program_state;

//Need forward declaration for this one since it is used so much.
LLVMValueRef process_expression(NodeExpression* expr, function_state function,
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
			fprintf(stderr, "ident not implemented as factor\n");
			break;
		case ARRIDX:
			fprintf(stderr, "arridx not implemented as factor\n");
			break;
		case NUM:
			return LLVMConstInt(LLVMInt32Type(), factor->fac.num->value, 0);
		case SUBEXPR:
			return process_expression(factor->fac.subexpr, function, program);
	}
	return NULL;
	//TODO: not, neg, call, ident, arridx, subexpr
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
void process_grouping(NodeGrouping* grouping, function_state function,
	program_state program) {
	if (grouping == NULL)
		return;
	if (grouping->type == STATEMENT) {
		process_statement(grouping->inner.statement, function, program);
	}
	process_grouping(grouping->next_grouping, function, program);
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
	table_init(global_symbols, H_STRING, NULL, NULL);
	program_state program_state = {mod, printf_, global_symbols};

	table_t* function_symbols = malloc_assert(sizeof(table_t));
	table_init(function_symbols, H_STRING, NULL, NULL);
	function_state main_state = {main_builder, function_symbols};

	process_grouping(root->grouping, main_state, program_state);

	table_destroy(main_state.symbols);
	free(main_state.symbols);

	table_destroy(program_state.symbols);
	free(program_state.symbols);

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
