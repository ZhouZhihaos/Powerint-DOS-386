#include <dos.h>
unsigned time(void) {
  extern struct TIMERCTL timerctl;
  unsigned int t;
  t = get_year() * get_mon_hex() * get_day_of_month() * get_hour_hex() * get_min_hex() * get_sec_hex() + timerctl.count + timerctl.next;
  t |= (int)timerctl.t0;
  t |= get_day_of_week();
  
  // printk("%08x\n", t);
  return t;
}
