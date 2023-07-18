#ifndef __PCONSOLE_HPP__
#define __PCONSOLE_HPP__
#include <window.hpp>
class PConsole : public Window {
 public:
  PConsole(struct SHTCTL* ctl, int x, int y, struct TASK *use_task);
  void putch(char ch);
  void scroll();
  void clear();
  void draw_box(int x,int y,int x1,int y1,char c);
  void move_cursor(int x,int y);
  struct TASK *use_task;
  int x,y,color;
 private:
  PSheetBase *cursur;
  
};
void register_tty(PConsole* p, struct TASK *task);
#endif