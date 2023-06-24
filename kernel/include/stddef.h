#ifndef __STDDEF__
#define __STDDEF__
#include <sys/types.h>
#ifndef NULL
#define NULL 0
#endif
#define offsetof(s,m) (size_t)&(((s *)0)->m)
typedef signed int ptrdiff_t;
typedef unsigned int size_t;
#ifndef __cplusplus
typedef unsigned short wchar_t;
#endif
typedef int errno_t;
typedef unsigned int uintptr_t;
#endif