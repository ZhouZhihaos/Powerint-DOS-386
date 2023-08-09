#include <dos.h>
// HTTP

HTTPGetHeader http_check(uint8_t* data, uint32_t size) {
  HTTPGetHeader hg;
  hg.ok = false;
  clean(hg.path, 13);
  if (memcmp(data, "GET ", 4) == 0) {
    hg.ok = true;
    data += 4;
    for (int i = 0; data[i] != ' ' && i < 12; i++) {
      hg.path[i] = data[i];
    }
  }
  return hg;
}

int __get_week(int year, int month, int day) {
  int a = (14 - month) / 12;
  int y = year - a;
  int m = month + 12 * a - 2;
  return (day + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7;
}
int get_week() {
  int year, month, day;
  year = get_year();
  month = get_mon_hex();
  day = get_day_of_month();
  return __get_week(year, month, day);
}
void GetNowDate(char* result) {
  /*UTC+8*/
  char* week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  char* _month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  int year, month, day, hour, minute, second;
  year = get_year();
  month = get_mon_hex();
  day = get_day_of_month();
  hour = get_hour_hex();
  minute = get_min_hex();
  second = get_sec_hex();
  io_cli();
  sprintf(result, "Date: %s, %02d %s %02d %02d:%02d:%02d GMT", week[get_week()],
          day, _month[month - 1], year, hour, minute, second);
  io_sti();
}
