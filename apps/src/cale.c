#include <string.h>
#include <syscall.h>
void clean(char* s, int l) {
  for (int i = 0; i < l; i++) {
    s[i] = 0;
  }
}
int atoi(char* str) {
  return strtol(str, NULL, 10);
}
int Week(int yy, int mm, int dd) {
  int year = yy;  //因为定义的是全局变量 所以就需要定义新变量来传递值
  int month = mm;  //当然 你也可以定义在main函数内然后用&引用传递
  int day = dd;
  if (mm < 3) {
    year -= 1;
    month += 12;
  }
  int y = year % 100;
  int c = year / 100;
  int d = day;
  int m = month;
  int w = (y + y / 4 + c / 4 - 2 * c + 13 * (m + 1) / 5 + d - 1) % 7;
  return w;
}
void cal(int year, int month) {
  int day = 0;
  if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 ||
      month == 10 || month == 12) {
    day = 31;
  } else if (month == 4 || month == 6 || month == 9 || month == 11) {
    day = 30;
  } else if (month == 2) {
    if (year % 4 == 0) {
      day = 29;
    } else {
      day = 28;
    }
  }
  char* buf = api_malloc(128);
  sprintf(buf, "     %dyear,%dmonth", year, month);
  print(buf);
  api_free(buf, 128);
  print("\n");
  print("Sun Mon Tue Wed Thu Fri Sat\n");
  buf = api_malloc(128);
  int week = 0;

  for (int i = 0; i < Week(year, month, 1); i++) {
    print("    ");
    week++;
  }
  week++;
  print("1   ");
  if (week % 7 == 0) {
    print("\n");
    week = 1;
  } else {
    week++;
  }
  for (int i = 2, k = week; i <= day; i++, k++) {
    sprintf(buf, "%d", i);
    print(buf);
    if (k % 7 == 0) {
      print("\n");
    } else {
      for (int j = 0; j < 3 - strlen(buf) + 1; j++) {
        print(" ");
      }
    }
    clean(buf, 128);
  }
  clean(buf, 128);
  api_free(buf, 128);
}
int main(int argc, char** argv) {
  char* buf = api_malloc(128);
  print("Year:");
  scan(buf, 128);
  int year = atoi(buf);
  clean(buf, 128);
  print("Month:");
  scan(buf, 128);
  int month = atoi(buf);
  clean(buf, 128);
  api_free(buf, 128);  //释放内存
  cal(year, month);
  return 0;
}