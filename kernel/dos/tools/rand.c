#include <dos.h>
#define UINT32_MAX 0xffffffff
#define INT32_MAX 0x7fffffff
static unsigned long next1 = 1;
unsigned int rand(void) {
  next1 = next1 * 1103515245 + 12345;
  return ((unsigned int)(next1));
}
void srand(unsigned long seed) {
  next1 = seed;
}
