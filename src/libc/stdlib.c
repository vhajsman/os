#include "kernel.h"

#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	printf("kernel: panic: abort()\n");
    kernel_panic(NULL, -1);
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
#endif
	while (1) { }
	__builtin_unreachable();
}

#include "memory/memory.h"

void* malloc(size_t size) {
	return kmalloc(size);
}

void free(void* ptr) {
	kfree(ptr);
}

void *calloc(size_t n, size_t size) {
	return kcalloc(n, size);
}

void *realloc(void* ptr, size_t size) {
	return krealloc(ptr, size);
}