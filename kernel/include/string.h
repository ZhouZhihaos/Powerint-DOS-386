#ifndef _STRING_H
#define _STRING_H
#include <ctypes.h>
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *s);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
void *memset(void *s, int c, size_t n);
long strtol(const char *nptr, char **endptr, int base);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *s, const void *ct, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
void strrev(char *s);
void F2S(double d, char *str, int l);
char *strchr(const char *s, int c);
char *strrchr(const char *s1, int ch);
void* memmove(void* dest, const void* src, int n);
void assert(int expression);
void *memchr(const void *s, int c, size_t n);
int stricmp(const char *s1, const char *s2);
int strincmp(const char *s1, const char *s2, size_t n);
#endif
