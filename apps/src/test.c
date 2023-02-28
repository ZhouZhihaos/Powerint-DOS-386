#include <stdio.h>
#include <syscall.h>
void t() {
  while(1) {
    get_mouse();
  }
}
int main() {
  char *s = malloc(1024*16);
  AddThread("t",t,s);
  while (1) {
    
  }
}