#ifndef __CACHE_H
#define __CACHE_H

#include "types.h"
#include "stdarg.h"

/*  
    This library allows you to cache functions.
    When function is called using fncache_call(), function cache region 
    is being searched for result of previous function instance with the
    same call params. If a match is found, fncache_call returns address
    to the result. If not, function is executed as usual and everything
    is being saved to cache, the the function result is returned.
*/

// Cached function parameter
struct CacheFnArg {
    void* data;
    size_t size;
};

// Cached function entry point
struct CacheFnEntry {
    unsigned long hash;
    void* result;
    size_t result_size;
    struct CacheFnArg* args;
    int argc;
    int cached;
};

// Function cache area in memory
struct CacheFn {
    struct CacheFnEntry* entries;
    int cache_size;
    int count;
};

// Creates cache are in memory and returns its address
struct CacheFn* fncache_init(int size);

// Frees memory area allocated for cache struct
void fncache_free(struct CacheFn* cache);

// Calls function and writes results to cache or returns cached result
void* fncache_call(struct CacheFn* cache, void* (*func)(size_t*, va_list), size_t* result_size, int argc, ...);

#define CF fncache_call

#endif