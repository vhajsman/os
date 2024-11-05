#include "cache.h"
#include "memory/memory.h"
#include "debug.h"

unsigned long hash_data(const void* data, size_t len) {
    const unsigned char* p = data;
    unsigned long hash = 5381;

    for(size_t i = 0; i < len; i++) 
        hash = ((hash << 5) + hash) + p[i];
    
    return hash;
}


struct CacheFn* fncache_init(int size) {
    struct CacheFn* cache = malloc(sizeof(struct CacheFn));
    if(cache == NULL)
        return NULL;

    cache->entries = malloc(sizeof(struct CacheFnEntry) * size);
    if(cache->entries == NULL) {
        debug_message("not enough memory for function cache entries", "cache", KERNEL_ERROR);

        free((struct CacheFn*) cache);
        return NULL;
    }

    for(int i = 0; i < size; i++) {
        cache->entries[i].cached = 0;
        cache->entries[i].result = NULL;
        cache->entries[i].args = NULL;
    }

    cache->cache_size = size;
    cache->count = 0;

    debug_message("initialized function cache with the size of ", "cache", KERNEL_MESSAGE);
    debug_number(size, 10);
    debug_append(" entries -> size on memory: ");
    debug_number(sizeof(struct CacheFn) + (sizeof(struct CacheFnEntry) + sizeof(struct CacheFnArg) * size), 10);
    debug_append(" bytes");

    return cache;
}

void fncache_free(struct CacheFn* cache) {
    if(cache == NULL)
        return;

    size_t size = sizeof(struct CacheFn) + (sizeof(struct CacheFnEntry) + sizeof(struct CacheFnArg) * cache->cache_size);
    
    for(int i = 0; i < cache->count; i++) {
        free(cache->entries[i].result);

        for(int ii = 0; ii < cache->entries->argc; ii++)
            free(cache->entries[i].args[ii].data);
        
        free(cache->entries[i].args);
    }

    free(cache->entries);
    free(cache);

    debug_message("freed memory allocated for function cache. total bytes freed: ", "cache", KERNEL_MESSAGE);
    debug_number(size, 10);
}

void* fncache_call(struct CacheFn* cache, void* (*func)(size_t*, va_list), size_t* result_size, int argc, ...) {
    if(cache == NULL || func == NULL)
        return NULL;

    va_list args;
    va_start(args, argc);

    unsigned long hash = 5381;

    struct CacheFnArg* arguments = malloc(sizeof(struct CacheFnArg) * argc);
    if(arguments == NULL)
        goto memory_insufficent;
    
    for(int i = 0; i < argc; i++) {
        void* arg = va_arg(args, void*);
        size_t arg_size = va_arg(args, size_t);

        arguments[i].data = malloc(arg_size);
        if(arguments[i].data == NULL) {
            for(int ii = 0; ii < i; ii++)
                free(arguments[i].data);
            
            free(arguments);

            goto memory_insufficent;
        }

        memcpy(arguments[i].data, arg, arg_size);
        arguments[i].size = arg_size;

        hash ^= hash_data(arg, arg_size);
    }

    va_end(args);

    for(int i = 0; i < cache->count; i++) {
        if(cache->entries[i].cached && cache->entries[i].hash == hash) {
            int match = 1;

            for(int ii = 0; ii < argc; ii++) {
                if(cache->entries[i].args[ii].size != arguments[ii].size || memcmp(cache->entries[i].args[ii].data, arguments[ii].data, arguments[ii].size) != 0) {
                    match = 0;
                    break;
                }
            }

            if(match) {
                *result_size = cache->entries->result_size;
                void* cached_result = malloc(*result_size);

                memcpy(cached_result, cache->entries[i].result, *result_size);

                for(int ii = 0; ii < argc; ii++)
                    free(arguments[ii].data);
                
                free(arguments);

                debug_message("return cached result", "cache", KERNEL_MESSAGE);
                return cached_result;
            }
        }
    }

    va_list args_copy;
    va_start(args_copy, argc);

    void* result = func(result_size, args_copy);

    va_end(args_copy);

    if(cache->count < cache->cache_size) {
        cache->entries[cache->count].hash = hash;
        cache->entries[cache->count].result_size = *result_size;

        cache->entries[cache->count].result = malloc(*result_size);
        if(cache->entries[cache->count].result == NULL) {
            return result;
        }
        
        memcpy(cache->entries[cache->count].result, result, *result_size);

        cache->entries[cache->count].args = arguments;
        cache->entries[cache->count].argc = argc;
        cache->entries[cache->count].cached = 1;
        
        cache->count++;
        
        return result;
    }

    for(int ci = 0; ci < argc; ci++)
        free(arguments[ci].data);
    free(arguments);

    return result;

memory_insufficent:
    debug_message("fncache_call(): not enough memory", "cache", KERNEL_ERROR);
    return NULL;
}

