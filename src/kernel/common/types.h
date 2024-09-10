#ifndef __TYPES_H
#define __TYPES_H

#define NULL ((void*)0)


typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long int u64;

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;

typedef u8              byte;
typedef u16             word;
typedef u32             dword;

typedef u64             size_t;

typedef enum {
    FALSE,
    TRUE
} BOOL;

#ifdef __cplusplus
    #define __bool_defined
#endif

#ifndef __bool_defined
#define __bool_defined
    #define __bool u8
    
    typedef __bool boolean_t;
    typedef __bool bool;
#endif

#endif