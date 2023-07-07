#include <box.hpp>
#include <psheet.hpp>
#include <pvec.hpp>

extern "C" void atexit() {}

WindowBox::WindowBox() {
  num = 0;
  box = new PVector<Box>();
}
void WindowBox::register_box(
    PSheetBase* ps,
    int x,
    int y,
    int x1,
    int y1,
    uint32_t val,
    void (*callback)(PSheetBase* ps, int x, int y, uint32_t val)) {
  Box b;
  b.callback = callback;
  b.x = x;
  b.y = y;
  b.y1 = y1;
  b.x1 = x1;
  b.ps = ps;
  b.val = val;
  b.flag = 1;
  for (int i = 0; i < this->box->get_size(); i++) {
    if ((*box)[i].flag == 0) {
      (*box)[i] = b;
      return;
    }
  }
  box->push_back(b);
}
PSheetBase* WindowBox::click_handle(struct SHTCTL* ctl, int x, int y) {
  int sx, sy;
  struct SHEET* sht = get_sheet_from_xy(ctl, x, y, &sx, &sy);
  uint32_t index = 0, flag = 0;
  for (; index < box->get_size(); index++) {
    if ((*box)[index].ps->get_sht() == sht && (*box)[index].flag) {
      flag = 1;
      break;
    }
  }
  if (!flag) {
    return nullptr;
  }
  Box b = (*box)[index];
  if (x >= sht->vx0 + b.x && x <= sht->vx0 + b.x1 && y >= sht->vy0 + b.y &&
      y <= sht->vy0 + b.y1) {
    b.callback(b.ps, sx, sy, b.val);
    return b.ps;
  }
  return nullptr;
}
void WindowBox::box_logout(PSheetBase* ps) {
  uint32_t index = 0;
  for (; index < box->get_size(); index++) {
    if ((*box)[index].ps->get_sht() == ps->get_sht() && (*box)[index].flag) {
      (*box)[index].flag = 0;
      break;
    }
  }
}
