#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <kernel/core/console.h>


#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

//int printf(const char* __restrict, ...);
//int putchar(int);
//int puts(const char*);

#define putchar putc

#ifdef __cplusplus
}
#endif

#endif