#ifndef __FCNTL__
#define __FCNTL__
#include <stddef.h>
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
typedef long ssize_t;

// off_t 类型的定义
#ifdef _WIN64
    typedef long long off_t;
#else
    typedef long off_t;
#endif
typedef unsigned short mode_t;

int open(const char *pathname, int flags, ...);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int close(int fd);
#endif