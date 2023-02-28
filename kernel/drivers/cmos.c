#include <dos.h>  
unsigned char read_cmos(unsigned char p) {
  unsigned char data;
  io_out8(cmos_index, p);
  data = io_in8(cmos_data);
  io_out8(cmos_index, 0x80);
  return data;
}
void write_cmos(unsigned char p, unsigned char data) {
  io_out8(cmos_index, p);
  io_out8(cmos_data, data);
  io_out8(cmos_index, 0x80);
  io_out8(cmos_data, 0);
}
unsigned int get_hour_hex() { return BCD_HEX(read_cmos(CMOS_CUR_HOUR)); }
unsigned int get_min_hex() { return BCD_HEX(read_cmos(CMOS_CUR_MIN)); }
unsigned int get_sec_hex() { return BCD_HEX(read_cmos(CMOS_CUR_SEC)); }
unsigned int get_day_of_month() { return BCD_HEX(read_cmos(CMOS_MON_DAY)); }
unsigned int get_day_of_week() { return BCD_HEX(read_cmos(CMOS_WEEK_DAY)); }
unsigned int get_mon_hex() { return BCD_HEX(read_cmos(CMOS_CUR_MON)); }
unsigned int get_year() {
  return (BCD_HEX(read_cmos(CMOS_CUR_CEN)) * 100) +
         BCD_HEX(read_cmos(CMOS_CUR_YEAR)) - 30 + 2010;
}
void write_cmos_time(unsigned int year, unsigned char mon, unsigned char day,
                     unsigned char hour, unsigned char min) {
  write_cmos(CMOS_CUR_HOUR, HEX_BCD(hour));
  write_cmos(CMOS_CUR_MIN, HEX_BCD(min));

  write_cmos(CMOS_CUR_YEAR, HEX_BCD(year % 100));
  // month
  write_cmos(CMOS_CUR_MON, HEX_BCD(mon));
  // day
  write_cmos(CMOS_MON_DAY, HEX_BCD(day));
}