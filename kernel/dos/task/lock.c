/*
 * Powerint DOS 386 LOCK的实现
 * Copyright (c) min0911_ 2022
 * @date 2022-6-8
 */
#include <dos.h>
static unsigned int lock_flag = 0;
bool cas(int *ptr, int old, int New) {
  io_cli();
  int old_value = *ptr;
  if (old_value == old) {
    *ptr = New;
    io_sti();
    return true;
  }
  io_sti();
  return false;
}
void lock() {
  while (!cas((int *)&lock_flag, 0, 1))
    ;
}
void unlock() {
  while (!cas((int *)&lock_flag, 1, 0))
    ;
}
