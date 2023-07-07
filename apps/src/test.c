#include <stdio.h>
#include <syscall.h>
#include <fcntl.h>
int main() {
  int fd = open("/autoexec.bat",O_RDONLY,0);
  char buf[512];
  read(fd,buf,512);
  close(fd);
  printf("%s\n",buf);
}