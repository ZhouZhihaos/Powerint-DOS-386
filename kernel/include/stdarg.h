#if (!defined(STDARG_H))

#define STDARG_H	1

#if (defined(__cplusplus))
	extern "C" {
#endif

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )
#define va_copy(d,s)	__builtin_va_copy((d),(s))
#define	va_list	char *

#if (defined(__cplusplus))
	}
#endif

#endif
