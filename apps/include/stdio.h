#ifndef _STDIO_H
#define _STDIO_H
#include <stdarg.h>
int printf(const char* format, ...);
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arg);
#endif