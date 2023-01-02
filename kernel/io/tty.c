#include <io.h>
void clear_tty(struct tty *tty) {
  int i;
  int j;
  for (i = 0; i < 160; i += 2) {
    for (j = 0; j < 25; j++) {
      *(char *)(tty->vram + j * 160 + i) = ' ';
      *(char *)(tty->vram + j * 160 + i + 1) = get_cons_color();
    }
  }
  tty->x = 0;
  tty->y = 0;
  tty->cursor.x = 0;
  tty->cursor.x = 0;
  tty->rx = 0;
  tty->ry = 0;
  Move_Cursor(tty->cursor.x, tty->cursor.x);
}
void screen_ne_tty(struct tty *tty) /*向下滚动一行*/
{
  // TODO:屏幕滚动！！！！
  int i;
  int j;
  int X = tty->x;
  for (i = 0; i < 160; i += 2) {
    for (j = 0; j < 24; j++) {
      *(char *)(tty->vram + j * 160 + i) =
          *(char *)(tty->vram + (j + 1) * 160 + i);
      *(char *)(tty->vram + j * 160 + i + 1) =
          *(char *)(tty->vram + (j + 1) * 160 + i + 1);
    }
  }
  for (i = 0; i < 160; i += 2) {
    *(char *)(tty->vram + 24 * 160 + i) = ' ';
    *(char *)(tty->vram + 24 * 160 + i + 1) = get_cons_color();
  }

  tty->cursor.x = 0;
  tty->x = 0;
  // s_ne_t++;
  tty->ry++;
  tty->rx = 0;
  Move_Cursor(tty->cursor.x, tty->cursor.y);
}
void putstr_tty(const char *str, int length, struct tty *tty) {
  int i;
  for (i = 0; i < length; i++) {
    if (tty->y == 24 && tty->x >= 160) {
      /*暂时什么也不做*/
      // TODO:屏幕滚动！！！！
      screen_ne_tty(tty);
    }
    if (str[i] == 0x0d) {
      continue;
    }
    putchar_tty(str[i], tty);
  }
}
void print_tty(const char *str, struct tty *tty) {
  putstr_tty(str, getlength(str), tty);
}
void putchar_tty(char ch, struct tty *tty) {

  if (tty->x == 160) {
    tty->y++;
    tty->ry++;
    tty->cursor.y++;
    tty->x = 0;
    tty->cursor.x = 0;
    tty->rx = 0;

    Move_Cursor(tty->cursor.x, tty->cursor.y);
  }
  if (ch == '\n') {
    if (tty->y == 24) {
      screen_ne_tty(tty);
      return;
    }
    tty->y++;
    tty->cursor.y++;
    tty->x = 0;
    tty->ry++;
    tty->rx = 0;
    tty->cursor.x = 0;
    Move_Cursor(tty->cursor.x, tty->cursor.y);
    return;
  } else if (ch == '\0') {
    // gotoxy(get_x()+1,get_y());
    return;
  } else if (ch == '\b') {
    if (tty->x == 0) {
      tty->cursor.y--;
      tty->y--;
      tty->cursor.x = 79;
      tty->ry--;
      tty->rx = 158;
      tty->x = 158;
      Move_Cursor(tty->cursor.x, tty->cursor.y);
      *(char *)(tty->vram + tty->y * 160 + tty->x) = ' ';
      return;
    }
    *(char *)(tty->vram + tty->y * 160 + tty->x - 2) = ' '; /*显示位置为第23行*/
    tty->x -= 2;
    tty->cursor.x--;
    Move_Cursor(tty->cursor.x, tty->cursor.y);
    return;
  } else if (ch == '\t') {
    //制表符
    // TODO:制表符
    // print("    ");
    return;
  }
  tty->cursor.x += 1;
  Move_Cursor(tty->cursor.x, tty->cursor.y);
  *(char *)(0xb8000 + tty->y * 160 + tty->x) = ch; /*显示位置为第23行*/
  tty->rx += 2;

  tty->x += 2;
}

void GotoXy_No_Safe_tty(int x1, int y1, struct tty *tty) {
  tty->cursor.x = x1;
  tty->cursor.y = y1;
  tty->x = x1 * 2;
  tty->y = y1;
  Move_Cursor(tty->cursor.x, tty->cursor.y);
}

void gotoxy_tty(int x1, int y1, struct tty *tty) {

  if (x1 >= 0 && y1 >= 0) {
    int x2 = x1;
    int y2 = y1;
    if (x1 <= 79 && y1 <= 24) {
      GotoXy_No_Safe_tty(x1, y1, tty);
      return;
    }
    if (x1 <= 79) {
      for (int i = 0; i < y1 - 24 - 1; i++) {
        screen_ne_tty(tty);
      }
      GotoXy_No_Safe_tty(x1, 24, tty);
      return;
    }

    if (x1 > 79) {
      y2 += x1 / 80 - 1;
      x2 = x1 % 80;
      if (y2 <= 24)
        gotoxy_tty(x2, y2 + 1, tty);
      else
        gotoxy_tty(x2, y2, tty);
    }
  } else { //负数
    //如果x1为负数 说明y1要--
    if (x1 < 0) {
      x1 += 80;
      y1--;
      gotoxy_tty(x1, y1, tty);
    }
    if (y1 < 0) {
      return;
    }
  }
}