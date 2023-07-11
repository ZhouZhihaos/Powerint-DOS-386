#include <math.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main() {
  char* p1 = malloc(4 * 1024 * 1024);
  FILE* fp = fopen("bitz.bin", "r");
  printf("fp->fileSize=%d\n", fp->fileSize);
  fclose(fp);
  return 0;
}
