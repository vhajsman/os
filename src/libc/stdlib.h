#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);

#ifdef __cplusplus
}
#endif

void *malloc(size_t size);
void free(void* ptr);
void *calloc(size_t n, size_t size);
void *realloc(void* ptr, size_t size);

#endif