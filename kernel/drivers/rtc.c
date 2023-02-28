/*
  * RTC.c : RTC 时钟驱动
  * Copyright (C) 2022 min0911_
  * For Powerint DOS 386 
  * TODO: 艹他奶奶的 中断收不到!！！！
*/
#include <drivers.h>
void rtc_handler() {
  send_eoi(0x8);
  while(1) printf("*");
}
void init_rtc() {
  ClearMaskIrq(8);
  io_out8(0x70,0x8a);
  io_out8(0x71,0x20);
  io_out8(0x70,0x8b);
  char prev_sec = io_in8(0x71);
  io_out8(0x70,0x8b);
  io_out8(0x71,prev_sec | 0x40);
}