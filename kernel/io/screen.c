#include <io.h>
extern struct tty *tty_default;
void clear() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->clear(tty_default);
  } else {
    task->TTY->clear(task->TTY);
  }
}
void printchar(char ch) {
  char ch1[2] = {ch, '\0'};
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->print(tty_default, ch1);
  } else {
    task->TTY->print(task->TTY, ch1);
  }
}
void putchar(char ch) {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->putchar(tty_default, ch);
  } else {
    task->TTY->putchar(task->TTY, ch);
  }
}
void screen_ne() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->screen_ne(tty_default);
  } else {
    task->TTY->screen_ne(task->TTY);
  }
}
int get_x() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    return tty_default->x;
  } else {
    return task->TTY->x;
  }
}
int get_y() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    return tty_default->y;
  } else {
    return task->TTY->y;
  }
}
int get_raw_y() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    return tty_default->Raw_y;
  } else {
    return task->TTY->Raw_y;
  }
}
int get_xsize() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    return tty_default->xsize;
  } else {
    return task->TTY->xsize;
  }
}
int get_ysize() {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    return tty_default->ysize;
  } else {
    return task->TTY->ysize;
  }
}
void print(const char *str) {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->print(tty_default, str);
  } else {
    task->TTY->print(task->TTY, str);
  }
}

void GotoXy_No_Safe(int x1, int y1) {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->MoveCursor(tty_default, x1, y1);
  } else {
    task->TTY->MoveCursor(task->TTY, x1, y1);
  }
}
void gotoxy(int x1, int y1) {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->gotoxy(tty_default, x1, y1);
  } else {
    task->TTY->gotoxy(task->TTY, x1, y1);
  }
}
void Text_Draw_Box(int x, int y, int x1, int y1, unsigned char color) {
  struct TASK *task = current_task();
  if (task->TTY->using1 != 1) {
    tty_default->Draw_Box(tty_default, x, y, x1, y1, color);
  } else {
    task->TTY->Draw_Box(task->TTY, x, y, x1, y1, color);
  }
}
