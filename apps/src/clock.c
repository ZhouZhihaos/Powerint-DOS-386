// Powerint DOS 16bits 2周年庆 移植程序 clock.bin
// Powerint DOS 386 & Simple OS 1周年庆
// zhouzhihao & min0911_
#include <math.h>
#include <syscall.h>

#define T_DrawBox(x, y, w, h, c) Text_Draw_Box((y), (x), (h) + y, (w) + x, (c))
static int table1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int table2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static char table3[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
int main(int argc, char **argv) {
  system("cls");
  goto_xy(1, 0);
  printf("Schedule");
  goto_xy(45, 0);
  printf("Clock");
  goto_xy(1, 1);
  printf("Sun   Mon   Tue   Wed   Thu   Fri   Sat   ");
  int y = get_year();
  int m = get_mon_hex();
  int ds;
  if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) {
    ds = table2[m - 1];
  } else {
    ds = table1[m - 1];
  }
  int c = y / 100;
  y = y % 100;
  int d = get_day_of_month();
  int start = (y + y / 4 + c / 4 - 2 * c + 13 * (m + 1) / 5 + 1 - 1) % 7;
  goto_xy(1 + start * 6, 2);
  int xy, xs, ys;
  for (int i = start; i != ds + start; i++) {
    if (i % 7 == 0) {
      printf("\n ");
    }
    printf("%02d    ", i - start + 1);
    if (i - start + 1 == d) {
      xy = get_xy();
      xs = xy >> 16;
      ys = xy & 0xffff;
    }
  }
  goto_xy(18, 9);
  printf("%s %02d", table3[m - 1], d);
  goto_xy(45, 2);
  printf("The current date is:");
  goto_xy(45, 5);
  printf("The current date is:");
  T_DrawBox(0, 0, 80, 25, 0xf0);
  T_DrawBox(1, 1, 42, 11, 0x0f);
  T_DrawBox(45, 1, 34, 11, 0x0f);
  T_DrawBox(xs - 6, ys, 3, 1, 0xf0);
  int s = -1;
  d = -1;
  for (;;) {
    int s1 = get_sec_hex();
    int d1 = get_day_of_month();
    if (s1 != s) {
      s = s1;
      goto_xy(45, 3);
      printf("%02d:%02d:%02d", get_hour_hex(), get_min_hex(), s);
      T_DrawBox(45, 3, 9, 1, 0x0f);
    }
    if (d1 != d) {
      d = d1;
      goto_xy(45, 6);
      printf("%04d\\%02d\\%02d", get_year(), get_mon_hex(), d);
      T_DrawBox(45, 6, 11, 1, 0x0f);
    }
  }
}
