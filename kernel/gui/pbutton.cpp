#include <pbutton.hpp>
#include <psheet.hpp>

void PButton::draw(char* name, color_t bc) {
  SDraw_Box(this->get_vram(), 0, 0, this->get_xsize(), this->get_ysize(),
            COL_000000, this->get_xsize());
  SDraw_Box(this->get_vram(), 1, 1, this->get_xsize() - 1,
            this->get_ysize() - 1, bc, this->get_xsize());
  int x = this->get_xsize() / 2 - strlen(name) * 8 / 2;
  int y = (this->get_ysize() - 1) / 2 - 8;
  while (*name) {
    SDraw_Char(this->get_vram(), x, y + 1, *(name++), COL_000000,
               this->get_xsize());
    x += 8;
  }
}
void pbutton_onclick(PSheetBase* ps, int x, int y, uint32_t val) {
  PButton* pbtn = (PButton*)val;
  if (!pbtn->flag) {
    pbtn->draw(pbtn->name, argb(0, 190, 190, 190));
    pbtn->refresh(0, 0, pbtn->get_xsize(), pbtn->get_ysize());
    pbtn->flag = 1;
  }
}
void pbutton_stay(PSheetBase* ps, int x, int y, uint32_t val) {
  PButton* pbtn = (PButton*)val;
  if (pbtn->flag) {
    pbtn->draw(pbtn->name, argb(0, 211, 211, 211));
    pbtn->refresh(0, 0, pbtn->get_xsize(), pbtn->get_ysize());
    pbtn->callback(pbtn, pbtn->val);
    pbtn->flag = 0;
  }
}
PButton::PButton(PSheetBase* ps,
                 char* name,
                 int x,
                 int y,
                 int xsize,
                 int ysize,
                 uint32_t val,
                 void (*callback)(PButton* pbtn, uint32_t val))
    : PSheetBase(ps, xsize, ysize, x, y, COL_TRANSPARENT) {
  this->updown(1);
  this->name = new char[strlen(name) + 1];
  this->callback = callback;
  this->val = val;
  strcpy(this->name, name);
  draw(name, argb(0, 211, 211, 211));
  this->regiser_box_left(0, 0, this->get_xsize(), this->get_ysize(),
                         (uint32_t)this, pbutton_onclick);
  this->regiser_box_stay(0, 0, this->get_xsize(), this->get_ysize(),
                         (uint32_t)this, pbutton_stay);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
}
PButton::~PButton() {
  delete[] this->name;
}