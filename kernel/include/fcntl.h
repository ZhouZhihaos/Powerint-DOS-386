#ifndef __Fcntl
#define __Fcntl
int open( const char * pathname,int flags, unsigned int mode);
int close(int fd);
unsigned int read (int fd, void *buf, unsigned int count);
unsigned int write(int fd, const void *buf, unsigned int nbyte);
#define O_RDONLY 0
#define O_WRONLY 0
#define O_CREAT  0
#define O_TRUNC  0
#define O_APPEND 0
#endif