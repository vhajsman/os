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

typedef __uint_8t uint8_t;
typedef __uint_16t uint16_t;
typedef __uint_32t uint32_t;
typedef __uint_64t uint64_t;

#define __int_8t   signed char
#define __int_16t  signed short
#define __int_32t  signed int
#define __int_64t  signed long int

typedef __int_8t int8_t;
typedef __int_16t int16_t;
typedef __int_32t int32_t;
typedef __int_64t int64_t;

#define __size_t __uint_64t
typedef __size_t size_t;

#define BYTE    char
#define WORD    short
#define DWORD   int
#define QWORD   long int

#endif
