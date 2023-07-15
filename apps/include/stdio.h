#ifndef _STDIO_H
#define _STDIO_H
#ifdef __cplusplus
extern "C" {
#endif
#define READ 0x2
#define WRITE 0x4
#define APPEND 0x8
#define BIN 0x0
#define PLUS 0x10
#define EOF -1
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define BUFSIZ (4096*2)
typedef struct FILE {
  unsigned int mode;
  unsigned int fileSize;
  unsigned char *buffer;
  unsigned int bufferSize;
  unsigned int p;
  char *name;
} FILE;
extern FILE *stdout;
extern FILE *stdin;
extern FILE *stderr;
#include <stdarg.h>
int printf(const char* format, ...);
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arg);
int puts(char *str);
char *gets(char *str);
FILE *fopen(char *filename,char *mode);
int fclose(FILE *fp);
int fseek(FILE *fp, int offset, int whence);
long ftell(FILE *stream);
unsigned int fwrite(const void *ptr, unsigned int size, unsigned int nmemb, FILE *stream);
unsigned int fread(void *buffer, unsigned int size, unsigned int count, FILE *stream);
int fgetc(FILE *stream);
int fputc(int ch,FILE *stream);
int fflush(FILE *stream);
char *fgets(char *str, int n, FILE *stream);
int fputs(const char *str, FILE *stream);
int fprintf (FILE* stream, const char*format, ...);
int feof(FILE *stream);
int ferror(FILE *stream);
int getc(FILE *stream);
int sscanf(const char * s, const char * fmt, ...);
int snprintf(char * s, unsigned n, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif