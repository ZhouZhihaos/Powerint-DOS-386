#include <math.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

int main() {
  malloc(4 * 1024 * 1024);
  char* p = malloc(4 * 1024 * 1024);
  *p = '1';
  printf("%c\n",*p);
  return 0;
}
