#ifndef __TYPES_H
#define __TYPES_H

#ifndef __defined_null
#define __defined_null 1
#define NULL ((void*) 0)
#endif


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

    #define true 1
    #define false 0
#endif

typedef unsigned short      wchar;
typedef unsigned int        dchar;
typedef unsigned long int   qchar;

#endif