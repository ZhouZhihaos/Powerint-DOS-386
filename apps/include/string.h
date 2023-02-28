// string.h
// By min0911_
#ifndef STRING_H_
#define STRING_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <ctypes.h>
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
size_t strlen(const char* s);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
void* memset(void* s, int c, size_t n);
long strtol(const char* nptr, char** endptr, int base);
int memcmp(const void* s1, const void* s2, size_t n);
void* memcpy(void* s, const void* ct, size_t n);
int strncmp(const char* s1, const char* s2, size_t n);
void strrev(char* s);
void F2S(double d, char* str, int l);
char *strchr(const char *s, int c);
char *strrchr(const char *s1, int ch);
#ifdef __cplusplus
}
#endif
#endif