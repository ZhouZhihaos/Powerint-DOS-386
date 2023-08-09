#include <stdio.h>
#include <fcntl.h>
#include <stddef.h>

int open(const char *pathname, int flags, ...) {
    FILE *file;
    
    if (flags & O_CREAT) {
        if (flags & O_TRUNC) {
            file = fopen(pathname, "w");
        } else {
            file = fopen(pathname, "a");
        }
    } else if (flags & O_RDWR) {
        file = fopen(pathname, "r+");
    } else if (flags & O_WRONLY) {
        file = fopen(pathname, "w");
    } else {
        file = fopen(pathname, "r");
    }
    if(!file) {
        return -1;
    }
    return (int)(uintptr_t)file;
}

ssize_t write(int fd, const void *buf, size_t count) {
    FILE *file = (FILE *)(uintptr_t)fd;
    return fwrite(buf, sizeof(char), count, file);
}

ssize_t read(int fd, void *buf, size_t count) {
    FILE *file = (FILE *)(uintptr_t)fd;
    return fread(buf, sizeof(char), count, file);
}

off_t lseek(int fd, off_t offset, int whence) {
    FILE *file = (FILE *)(uintptr_t)fd;
    fseek(file, offset, whence);
    return ftell(file);
}
int close(int fd) {
    FILE *file = (FILE *)(uintptr_t)fd;
    
    if (fclose(file) == 0) {
        return 0; // 关闭成功，返回0
    } else {
        return -1; // 关闭失败，返回-1
    }
}
