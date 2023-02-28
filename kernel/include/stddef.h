#ifndef __STDDEF__
#define __STDDEF__
#include <sys/types.h>
#define NULL 0
#define offsetof(s,m) (size_t)&(((s *)0)->m)
typedef signed int ptrdiff_t;
typedef unsigned int size_t;
typedef unsigned short wchar_t;
typedef int errno_t;
#endif