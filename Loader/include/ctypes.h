#ifndef _CTYPES_H
#define _CTYPES_H
#define bool int
#define true 1
#define false 0
#ifndef NULL
#  ifdef __cplusplus
#   define NULL nullptr
#  else
#   define NULL ((void*)0)
#  endif
#endif
typedef unsigned int size_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#endif
