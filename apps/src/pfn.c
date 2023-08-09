#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  printf("hello, world! %s\n",getenv("network"));
  return 0;
}
