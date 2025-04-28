#ifndef __STDINT_H
#define __STDINT_H

#ifndef __defined_null
#define __defined_null 1
    #define NULL ((void*) 0)
#endif

#ifndef __defined_nullptr
#define __defined_nullptr 1
    #define nullptr ((void*) 0)
#endif

#define __size_t __uint_64t
typedef __size_t size_t;

#define BYTE    char
#define WORD    short
#define DWORD   int
#define QWORD   long int

// ===========================================
//  UNSIGNED INTEGERS
// ===========================================

#define __uint_8t   unsigned char
#define __uint_16t  unsigned short
#define __uint_32t  unsigned int
#define __uint_64t  unsigned long int

typedef __uint_8t uint8_t;
typedef __uint_16t uint16_t;
typedef __uint_32t uint32_t;
typedef __uint_64t uint64_t;

#define UINT8_MAX   255
#define UINT16_MAX  65535
#define UINT32_MAX  4294967295U
#define UINT64_MAX  18446744073709551615ULL

#define UINT8_C(v)  (v ## U)
#define UINT16_C(v) (v ## U)
#define UINT32_C(v) (v ## U)
#define UINT64_C(v) (v ## ULL)


// ===========================================
//  SIGNED INTEGERS
// ===========================================

#define __int_8t   signed char
#define __int_16t  signed short
#define __int_32t  signed int
#define __int_64t  signed long int

typedef __int_8t int8_t;
typedef __int_16t int16_t;
typedef __int_32t int32_t;
typedef __int_64t int64_t;

#define INT8_MAX   127
#define INT16_MAX  32767
#define INT32_MAX  2147483647
#define INT64_MAX  9223372036854775807LL

#define INT8_MIN   -128
#define INT16_MIN  -32768
#define INT32_MIN  (-INT32_MAX-1)
#define INT64_MIN  (-INT64_MAX-1)

#define INT8_C(v)  (v)
#define INT16_C(v) (v)
#define INT32_C(v) (v)
#define INT64_C(v) (v ## LL)


#endif
