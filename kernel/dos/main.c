#include <dos.h>
void KernelMain(void) {
  check();
  sysinit();
  for (;;)
    ;
}
