#include <dos.h>
struct List *tty_list;
struct tty *tty_default;
static void tty_print(struct tty *res, const char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (res->y == res->ysize && res->x >= res->xsize) {
      res->screen_ne(res);
    }
    if (GetCHorEN(&string[i]) == 1) {
      res->putchar(res, *(short *)&string[i]);
      i++;
    } else {
      res->putchar(res, string[i]);
    }
  }
}
static void tty_gotoxy(struct tty *res, int x, int y) {
  if (x >= 0 && y >= 0) {
    int x2 = x;
    int y2 = y;
    if (x <= res->xsize - 1 && y <= res->ysize - 1) {
      res->MoveCursor(res, x, y);
      return;
    }
    if (x <= res->xsize - 1) {
      for (int i = 0; i < y - res->ysize + 1; i++) {
        res->screen_ne(res);
      }
      res->MoveCursor(res, x, res->ysize - 1);
      return;
    }
    if (x > res->xsize - 1) {
      y2 += x / res->xsize - 1;
      x2 = x % res->xsize;
      if (y2 <= res->ysize - 1)
        tty_gotoxy(res, x2, y2 + 1);
      else
        tty_gotoxy(res, x2, y2);
    }
  } else {
    if (x < 0) {
      x += res->xsize;
      y--;
      tty_gotoxy(res, x, y);
    }
    if (y < 0) {
      return;
    }
  }
}
void init_tty() {
  tty_list = NewList();
  tty_default =
      tty_alloc(0xb8000, 80, 25, putchar_TextMode, MoveCursor_TextMode,
                clear_TextMode, screen_ne_TextMode, Draw_Box_TextMode);
}
struct tty *tty_alloc(void *vram, int xsize, int ysize,
                      void (*putchar)(struct tty *res, int c),
                      void (*MoveCursor)(struct tty *res, int x, int y),
                      void (*clear)(struct tty *res),
                      void (*screen_ne)(struct tty *res),
                      void (*Draw_Box)(struct tty *res, int x, int y, int x1,
                                       int y1, unsigned char color)) {
  struct tty *res = (struct tty *)page_kmalloc(sizeof(struct tty));
  res->using1 = 1;
  res->x = 0;
  res->y = 0;
  res->vram = vram;
  res->xsize = xsize;
  res->ysize = ysize;
  res->putchar = putchar;
  res->MoveCursor = MoveCursor;
  res->clear = clear;
  res->screen_ne = screen_ne;
  res->Draw_Box = Draw_Box;
  res->gotoxy = tty_gotoxy;
  res->print = tty_print;
  res->color = 0x07;
  AddVal((int)res, tty_list);
  return res;
}
void tty_free(struct tty *res) {
  int i;
  for (i = 0; FindForCount(i, tty_list) != (int)res; i++)
    ;
  DeleteVal(i, tty_list);
  free((void *)res);
  return;
}
struct tty *tty_set(struct TASK *task, struct tty *res) {
  if (res->using1 == 1) {
    struct tty *old = task->TTY;
    task->TTY = res;
    return old;
  }
  return NULL;
}
struct tty *tty_set_default(struct tty *res) {
  if (res->using1 == 1) {
    struct tty *old = tty_default;
    tty_default = res;
    return old;
  }
  return NULL;
}
void tty_set_reserved(struct tty *res, unsigned int reserved1,
                      unsigned int reserved2, unsigned int reserved3,
                      unsigned int reserved4) {
  res->reserved[0] = reserved1;
  res->reserved[1] = reserved2;
  res->reserved[2] = reserved3;
  res->reserved[3] = reserved4;
}
