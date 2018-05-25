#include "tree.h"
#include <assert.h>

void* malloc_assert(size_t size) {
	void* mem = malloc(size);
	assert(mem != NULL);
	return mem;
}


