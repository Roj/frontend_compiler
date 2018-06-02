#include "tree.h"
#include <assert.h>

void* malloc_assert(size_t size) {
	void* mem = malloc(size);
	assert(mem != NULL);
	return mem;
}

int get_num_arguments(NodeArguments* args) {
	int i = 0;
	while (args) {
		i++;
		args = args->next_arg;
	}
	return i;
}

int get_num_params(NodeParams* params) {
	int i = 0;
	while (params) {
		i++;
		params = params->next_param;
	}
	return i;
}

//Something a little bit annoying is that arguments can be pointers to memory,
//but we are passing them as expressions. An identifier inside an expression 
//has the value copied (i.e. the pointer is de-referenced).
//If the expression is just an identifier, we can take it as a pointer, without
//de-referencing it, given that the function allows it.
//With this we are not allowing any pointer arithmetic.
char* get_name_if_arg_is_identifier(NodeArguments* arg) {
	if (! arg->is_expr)
		return NULL;
	NodeExpression* expr = arg->arg.expr;
	if (expr->exp_prime != NULL 
		|| expr->term->term_prime != NULL
		|| expr->term->factor->type != IDENT)
		return NULL;
	return expr->term->factor->fac.id;
}
