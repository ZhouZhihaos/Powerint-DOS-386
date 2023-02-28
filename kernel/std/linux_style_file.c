#define NOREAD
#include <dos.h>
#include <fcntl.h>
typedef struct OPEN {
  unsigned char* buf;
  unsigned int p;
  unsigned int size;
  unsigned char* path;
  unsigned arrsz;
  int wg;
} OPEN;
int close(int fd) {
  printk("Close(%08x)\n", fd);
  OPEN* fp = (OPEN*)fd;
  printk("size=%d\n", fp->size);
  if (fp->wg) {
    EDIT_FILE(fp->path, fp->buf, fp->size, 0);
  }
  free(fp->buf);
  free(fp->path);
  free(fp);
  return 0;
}

int open(const char* pathname, int flags, unsigned int mode) {
  printf("OPEN:%s\n", pathname);
  OPEN* fp;
  fp = malloc(sizeof(OPEN));
  FILE* fp1;
  fp1 = fopen(pathname, "wb");
  fp->size = fp1->size;
  fp->buf = malloc(fp->size);
  fp->path = malloc(strlen(fp1->path) + 1);
  fp->arrsz = fp->size;
  strcpy(fp->path, pathname);
  memcpy(fp->buf, fp1->buf, fp->size);
  fclose(fp1);
  fp->p = 0;
  fp->wg = 0;
  return (int)fp;
}

unsigned int read(int fd, void* buf, unsigned int count) {
  OPEN* fp = (OPEN*)fd;
  int i;
  for (i = 0; i < count; i++) {
    if (fp->p >= fp->size) {
      break;
    }
    ((unsigned char*)buf)[i] = fp->buf[fp->p++];
  }
  return i;
}
void rc(int fd, unsigned char c) {
  OPEN* fp = (OPEN*)fd;
  if (fp->size < fp->arrsz) {
    fp->buf[fp->p++] = c;
  } else {
    fp->arrsz += 4096;
    unsigned char* re = malloc(fp->arrsz);
    memcpy(re, fp->buf, fp->size);
    free(fp->buf);
    fp->buf = re;
    fp->buf[fp->p++] = c;
  }
  fp->size++;
  fp->wg = 1;
}
unsigned int write(int fd, const void* buf, unsigned int nbyte) {
  OPEN* fp = (OPEN*)fd;
  printk("fp=%08x\n", fp);
  for (int i = 0; i < nbyte; i++) {
    rc(fd, ((unsigned char*)buf)[i]);
  }
  return nbyte;
}
unsigned int lseek(int fd, unsigned int offset, int whence) {
  OPEN* fp = (OPEN*)fd;
  if (whence == 0) {
    fp->p = offset;
  } else if (whence == 1) {
    fp->p += offset;
  } else if (whence == 2) {
    fp->p = fp->size + offset;
  } else {
    return -1;
  }
  return 0;
}