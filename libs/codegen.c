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

#define DUMP 1

typedef struct function_state {
	LLVMBuilderRef builder;
} function_state;
typedef struct program_state {
	LLVMModuleRef mod;
	LLVMValueRef printf_ref;
} program_state;

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
LLVMValueRef process_factor(NodeFactor* factor, function_state function,
	program_state program) {
	if (factor->type == NUM)
		return LLVMConstInt(LLVMInt32Type(), factor->fac.num->value, 0);
	//TODO: not, neg, call, ident, arridx, subexpr
	return NULL;
}
LLVMValueRef process_term(NodeTerm* term, function_state function, 
	program_state program) {
	//TODO: process_factor (op) process_termprime
	return process_factor(term->factor, function, program);
}
LLVMValueRef process_expression(NodeExpression* expr, function_state function,
	program_state program) {
	//TODO: process_term (op) process_expressionprime
	return process_term(expr->term, function, program);
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
	if (grouping->type == STATEMENT) {
		process_statement(grouping->inner.statement, function, program);
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

	program_state program_state = {mod, printf_};
	function_state main_state = {main_builder};

	process_grouping(root->grouping, main_state, program_state);

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
	const char* target_triple = "x86_64-pc-linux-gnu";
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
