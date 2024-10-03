#ifndef __STDINT_H
#define __STDINT_H

//#include "kernel/common/types.h"

#ifndef __defined_null
#define __defined_null 1
#define NULL ((void*) 0)
#endif

#define __uint_8t   unsigned char
#define __uint_16t  unsigned short
#define __uint_32t  unsigned int
#define __uint_64t  unsigned long int

typedef uint8_t __uint_8t;
typedef uint16_t __uint_16t;
typedef uint32_t __uint_32t;
typedef uint64_t __uint_64t;

#define __int_8t   signed char
#define __int_16t  signed short
#define __int_32t  signed int
#define __int_64t  signed long int

typedef int8_t __int_8t;
typedef int16_t __int_16t;
typedef int32_t __int_32t;
typedef int64_t __int_64t;

#define __size_t __uint_64t
typedef __size_t size_t;

#define BYTE    char
#define WORD    short
#define DWORD   int
#define QWORD   long int

#endif
