#include <drivers.h>
void gensound(short ,int);
void beep(int point, int notes, int dup) {
  //蜂鸣器
  short low[7] = {138, 147, 165, 175, 196, 220, 247};
  short high[7] = {262, 294, 330, 349, 392, 440, 494};
  short middle[7] = {524, 587, 659, 698, 784, 880, 988};
  //调用蜂鸣器驱动程序
  if (point == 1) {
    gensound(low[notes - 1], dup);
  } else if (point == 2) {
    gensound(middle[notes - 1], dup);
  } else if (point == 3) {
    gensound(high[notes - 1], dup);
  }
  return;
}