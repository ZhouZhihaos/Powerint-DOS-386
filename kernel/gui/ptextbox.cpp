#include <psheet.hpp>
#include <textbox.hpp>
PTextBox* t = nullptr;
extern "C" int sc2a(int sc);
void ptextbox_onclick(PSheetBase* ps, int x, int y, uint32_t val) {
  t = (PTextBox*)val;
}
void ptextbox_key(char ch, uint32_t val) {
  ch = sc2a(ch);
  if ((uint32_t)t == val) {
    if (ch == '\b') {
      if (t->x == 1) {
        return;
      }
      t->m->buf[--t->m->len] = 0;
      t->x -= 8;
      SDraw_Box(t->get_vram(), t->x, t->y, t->x + 8, t->y + 16, COL_FFFFFF,
                t->get_xsize());
      t->refresh(t->x, t->y, t->x + 8, t->y + 16);
      return;
    }
    if (t->x + 8 >= t->get_xsize()) {
      return;
    }
    SDraw_Box(t->get_vram(), t->x, t->y, t->x + 8, t->y + 16, COL_FFFFFF,
              t->get_xsize());
    SDraw_Char(t->get_vram(), t->x, t->y, ch, COL_000000, t->get_xsize());
    t->refresh(t->x, t->y, t->x + 8, t->y + 16);
    t->x += 8;
    mstr_add_char(t->m, ch);
  }
}
PTextBox::PTextBox(PSheetBase* ps, int x, int y, int xsize, int ysize)
    : PSheetBase(ps, xsize, ysize, x, y, COL_TRANSPARENT) {
  SDraw_Box(this->get_vram(), 0, 0, this->get_xsize(), this->get_ysize(),
            COL_000000, this->get_xsize());
  SDraw_Box(this->get_vram(), 1, 1, this->get_xsize() - 1,
            this->get_ysize() - 1, COL_FFFFFF, this->get_xsize());
  this->updown(1);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
  this->register_key_press(ptextbox_key, (uint32_t)this);
  this->regiser_box_left(0, 0, this->get_xsize(), this->get_ysize(),
                         (uint32_t)this, ptextbox_onclick);
  this->m = mstr_init();
  this->x = 1;
  this->y = 1;
}
char* PTextBox::Text() {
  return mstr_get(this->m);
}
PTextBox::~PTextBox() {
  mstr_free(this->m);
}