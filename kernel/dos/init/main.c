#include <dos.h>
void check(void);
void KernelMain(void) {
  check();
  sysinit();
  for (;;)
    ;
}
