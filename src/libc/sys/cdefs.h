#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H 1

#define __cubebox_libc 1

#ifdef __cplusplus
    void* operator new(size_t size);
    void operator delete(void* ptr);
#endif

#endif