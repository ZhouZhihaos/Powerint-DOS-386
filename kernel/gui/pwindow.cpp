#include <psheet.hpp>
#include <window.hpp>
extern PSheetBase* mouse_sheet;
extern struct SHTCTL* ctl;
void Draw_Circle(vram_t* vram, int xsize, int x, int y, int r, color_t col) {
  int i, j;
  int flag;
  for (i = 0; i <= r; i++) {
    for (j = 0; j <= r; j++) {
      flag = (i * i + j * j) <= (r * r);
      if (flag) {
        vram[(y + j) * xsize + (x + i)] = col;
        vram[(y + j) * xsize + (x - i)] = col;
        vram[(y - j) * xsize + (x + i)] = col;
        vram[(y - j) * xsize + (x - i)] = col;
      }
    }
  }
}
void window_drop(PSheetBase* ps, int x, int y, uint32_t val) {
  // printk("a?\n");
  if (ps->get_left_last() == ps) {
    int rx = ps->get_sht()->vx0 + mdec.x;
    int ry = ps->get_sht()->vy0 + mdec.y;
    ps->slide(rx, ry);
  } else {
    ps->updown(ctl->top - 1);
  }
}
void window_close(PSheetBase* ps, int x, int y, uint32_t val) {
  Window* w = (Window*)val;

  delete w;
}
void Window::draw(char* title) {
  SDraw_Box(this->get_vram(), 0, 0, this->get_xsize(), this->get_ysize(),
            argb(0, 89, 117, 183), this->get_xsize());
  SDraw_Box(this->get_vram(), 2, 28, this->get_xsize() - 2,
            this->get_ysize() - 2, argb(0, 232, 232, 232), this->get_xsize());
  // Draw_Line(this->get_vram(), this->get_xsize() - 27, 0, get_xsize()-1, 27,
  //           argb(0, 255, 0, 0), this->get_xsize());
  // Draw_Line(this->get_vram(), this->get_xsize() - 1, 0, get_xsize() - 27, 27,
  //           argb(0, 255, 0, 0), this->get_xsize());
  Draw_Circle(this->get_vram(), this->get_xsize(), this->get_xsize() - 15, 14,
              10, argb(0, 255, 0, 0));
  int x = 5;
  while (*title) {
    SDraw_Char(this->get_vram(), x, 6, *(title++), COL_FFFFFF,
               this->get_xsize());
    x += 8;
  }
}
Window::Window(struct SHTCTL* ctl,
               char* title,
               int x,
               int y,
               int xsize,
               int ysize)
    : PSheetBase(ctl, xsize, ysize, x, y, COL_TRANSPARENT) {
  this->updown(ctl->top);
  this->draw(title);
  this->regiser_box_left(0, 0, this->get_xsize(), 28, (uint32_t)this,
                         window_drop);
  this->regiser_box_left(this->get_xsize() - 28, 0, this->get_xsize(), 28,
                         (uint32_t)this, window_close);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
}
Window::Window(struct SHTCTL* ctl,
               char* title,
               int x,
               int y,
               int xsize,
               int ysize,
               void (*close)(PSheetBase* ps, int x, int y, uint32_t val))
    : PSheetBase(ctl, xsize, ysize, x, y, COL_TRANSPARENT) {
  this->updown(ctl->top);
  this->draw(title);
  this->regiser_box_left(0, 0, this->get_xsize(), 28, (uint32_t)this,
                         window_drop);
  this->regiser_box_left(this->get_xsize() - 28, 0, this->get_xsize(), 28,
                         (uint32_t)this, close);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
}
