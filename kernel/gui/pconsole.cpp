#include <pconsole.hpp>
#include <window.hpp>
extern "C" int sc2a(int sc);
extern color_t color_table[16];
void pcons_key_press(char ch, uint32_t val) {
  // printk("this.\n");
  PConsole* p = (PConsole*)val;
  fifo8_put(TaskGetKeyfifo(p->use_task), (uint8_t)ch);
}
PConsole::PConsole(struct SHTCTL* ctl, int x, int y, struct TASK *use_task)
    : Window(ctl, "console", x, y, 644, 431) {
  SDraw_Box(this->get_vram(), 2, 28, 2 + 8 * 80, 28 + 16 * 25, COL_000000,
            this->get_xsize());
  this->cursur = new PSheetBase(this, 8, 16, 2, 28, COL_TRANSPARENT);
  SDraw_Box(cursur->get_vram(), 0, 0, 8, 16, COL_FFFFFF, cursur->get_xsize());
  this->cursur->refresh(0, 0, 8, 16);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
  this->x = 2;
  this->y = 28;
  this->register_key_press(pcons_key_press, (uint32_t)this);
  this->use_task = use_task;
  this->color = 0x0f;
}
static void copy_char(vram_t* vram,
                      int off_x,
                      int off_y,
                      int x,
                      int y,
                      int x1,
                      int y1,
                      int xsize) {
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      vram[(y + i + off_y) * xsize + (j + x + off_x)] =
          vram[(y1 + i + off_y) * xsize + (j + x1 + off_x)];
    }
  }
}
void PConsole::scroll() {
  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 80; j++) {
      copy_char(this->get_vram(), 2, 28, j * 8, i * 16, j * 8, (i + 1) * 16,
                this->get_xsize());
    }
  }
  SDraw_Box(this->get_vram(), 2, 28 + 24 * 16, 2 + 80 * 8, 28 + 25 * 16,
            color_table[this->color >> 4], this->get_xsize());
  this->y = 28 + 24 * 16;
  this->x = 2;
  this->cursur->slide(this->x, this->y);
  this->refresh(2, 28, 2 + 8 * 80, 28 + 16 * 25);
}

void PConsole::putch(char ch) {
  if (this->x == 2 + 8 * 80) {
    this->x = 2;
    this->y += 16;
  }
  if (this->y >= 28 + 16 * 25) {
    scroll();
  }
  if (ch == '\b') {
    if (this->x == 2) {
      if (this->y != 28) {
        this->x = 2 + 8 * 80;
        this->y -= 16;
        this->putch('\b');
        return;
      }
      return;
    }
    this->x -= 8;
    SDraw_Box(this->get_vram(), this->x, this->y, this->x + 8, this->y + 16,
              color_table[this->color >> 4], this->get_xsize());
    this->refresh(this->x, this->y, this->x + 8, this->y + 16);
    this->cursur->slide(this->x, this->y);
    return;
  }
  if (ch == '\n') {
    if (this->y + 16 >= 28 + 16 * 25) {
      scroll();
      return;
    }
    this->x = 2;
    this->y += 16;
    this->cursur->slide(this->x, this->y);
    return;
  }
  if (this->x + 8 > 2 + 8 * 80) {
    return;
  }
  if (ch == '\t') {
    this->putch(' ');
    this->putch(' ');
    this->putch(' ');
    this->putch(' ');
    return;
  }
  SDraw_Box(this->get_vram(), this->x, this->y, this->x + 8, this->y + 16,
            color_table[this->color >> 4], this->get_xsize());
  SDraw_Char(this->get_vram(), this->x, this->y, ch,
             color_table[this->color & 0xf], this->get_xsize());
  this->refresh(this->x, this->y, this->x + 8, this->y + 16);
  this->x += 8;

  this->cursur->slide(this->x, this->y);
}
void PConsole::clear() {
  SDraw_Box(this->get_vram(), 2, 28, 2 + 8 * 80, 28 + 16 * 25, COL_000000,
            this->get_xsize());
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
  this->x = 2;
  this->y = 28;
  this->cursur->slide(this->x, this->y);
}
void PConsole::draw_box(int x, int y, int x1, int y1, char c) {
  for (int i = y + 28; i <= y1 + 28; i++) {
    for (int j = x + 2; j <= x1 + 2; j++) {
      if (this->get_vram()[i * this->get_xsize() + j] ==
          color_table[this->color & 0xf]) {
        this->get_vram()[i * this->get_xsize() + j] = color_table[c & 0xf];
      } else {
        this->get_vram()[i * this->get_xsize() + j] = color_table[c >> 4];
      }
    }
  }
  this->refresh(x + 2, y + 28, x1 + 2, y1 + 28);
}
void PConsole::move_cursor(int x, int y) {
  this->x = 2 + x * 8;
  this->y = 28 + y * 16;
  this->cursur->slide(this->x, this->y);
}
// tty api 实现

void screen_ne_pgui(struct tty* res) {
  PConsole* p = (PConsole*)res->vram;
  p->scroll();
  res->x = (p->x - 2) / 8;
  res->y = (p->y - 28) / 16;
}
void putchar_pgui(struct tty* res, int c) {
  PConsole* p = (PConsole*)res->vram;
  p->color = res->color;
  //printk("%08x ",res->color);
  p->putch(c);
  res->x = (p->x - 2) / 8;
  res->y = (p->y - 28) / 16;
}
void clear_pgui(struct tty* res) {
  PConsole* p = (PConsole*)res->vram;
  p->clear();
  res->x = (p->x - 2) / 8;
  res->y = (p->y - 28) / 16;
}
void MoveCursor_pgui(struct tty* res, int x, int y) {
  PConsole* p = (PConsole*)res->vram;
  p->move_cursor(x, y);
  res->x = (p->x - 2) / 8;
  res->y = (p->y - 28) / 16;
  return;
}
void Draw_Box_pgui(struct tty* res,
                   int x,
                   int y,
                   int x1,
                   int y1,
                   unsigned char color) {
  PConsole* p = (PConsole*)res->vram;
  p->draw_box(x * 8, y * 16, x1 * 8, y1 * 16, color);
  res->x = (p->x - 2) / 8;
  res->y = (p->y - 28) / 16;
}
void register_tty(PConsole* p, struct TASK *task) {
  struct tty* tty_g = tty_alloc((void*)p, 80, 25, putchar_pgui, MoveCursor_pgui,
                                clear_pgui, screen_ne_pgui, Draw_Box_pgui);
  tty_set_reserved(tty_g, (unsigned int)0, (unsigned int)task, 0, 0);
  tty_set_default(tty_g);
  tty_set(task, tty_g);
}