#include <math.h>
#include <string.h>
#include <syscall.h>
bool is_pfn(int num) {
  //判断是否是完美数
  int sum = 0;
  for (int i = 1; i <= num; i++) {
    if (num % i == 0) {
      sum += i;
    }
  }
  if (sum - num == num) {
    return true;
  } else {
    return false;
  }
}
void clean(char* s, int l) {
  for (int i = 0; i < l; i++) {
    s[i] = 0;
  }
}
int main(int argc, char** argv) {
  char* buf = api_malloc(128);
  for (int i = 1;; i++) {
    if (is_pfn(i)) {
      sprintf(buf, "%d", i);
      print(buf);
      print("\n");
    }
  }
  return 0;
}
