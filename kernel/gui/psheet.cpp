extern "C" {
#include <dos.h>
}
#include <box.hpp>
#include <psheet.hpp>
#include <pvec.hpp>
extern WindowBox* click_left;
extern WindowBox* click_right;
extern WindowBox* stay_box;

PSheetBase::PSheetBase(struct SHTCTL* ctl,
                       int xsize,
                       int ysize,
                       int init_x,
                       int init_y,
                       int col_ent) {  // 创建，非窗口内窗口
  this->sht = sheet_alloc(ctl);
  this->vram = (vram_t*)malloc(xsize * ysize * sizeof(vram_t));
  this->draw_vram = (vram_t*)malloc(xsize * ysize * sizeof(vram_t));
  sheet_setbuf(this->sht, this->vram, xsize, ysize, col_ent);
  this->now_x = init_x;
  this->now_y = init_y;
  this->x_size = xsize;
  this->y_size = ysize;
  this->mode = 1;
  this->stl = shtctl_init(this->vram, xsize, ysize);
  this->outermost_sht = sheet_alloc(this->stl);
  this->reg_flag = 0;
  sheet_setbuf(this->outermost_sht, this->draw_vram, this->x_size, this->y_size,
               0x12ffffff);
  sheet_slide(this->outermost_sht, 0, 0);
  sheet_updown(this->outermost_sht, 0);
  sheet_slide(this->sht, init_x, init_y);
}
PSheetBase::PSheetBase(PSheetBase* ps,
                       int xsize,
                       int ysize,
                       int x,
                       int y,
                       int col_ent)  // 创建窗口内窗口
{
  this->x_size = xsize;
  this->y_size = ysize;
  this->now_x = x;
  this->now_y = y;
  this->dad = ps;
  this->mode = 2;
  this->vram = (vram_t*)malloc(xsize * ysize * sizeof(vram_t));
  this->draw_vram = (vram_t*)malloc(xsize * ysize * sizeof(vram_t));
  this->sht = ps->add(this, this->vram, col_ent);
  this->stl = shtctl_init(this->vram, xsize, ysize);
  this->outermost_sht = sheet_alloc(this->stl);
  this->reg_flag = 0;
  sheet_setbuf(this->outermost_sht, this->draw_vram, this->x_size, this->y_size,
               0x12ffffff);
  sheet_slide(this->outermost_sht, 0, 0);
  sheet_updown(this->outermost_sht, 0);
  sheet_slide(this->sht, x, y);
  sheet_updown(this->sht, 1);
  this->refresh(0, 0, this->get_xsize(), this->get_ysize());
}
PSheetBase::~PSheetBase() {
  for (int i = 0; i < p_sheet.get_size(); i++) {
    if (p_sheet[i] != nullptr) {
      delete p_sheet[i];
    }
  }
  if (this->mode == 1) {
    if (reg_flag) {
      click_left->box_logout(this);
    } else if (reg_flag_right) {
      click_right->box_logout(this);
    } else if (reg_flag_stay) {
      stay_box->box_logout(this);
    }
  }
  int vx0, vy0;
  vx0 = sht->vx0;
  vy0 = sht->vy0;
  sheet_free(outermost_sht);
  ctl_free(stl);
  sheet_free(sht);
  free(this->vram);
  free(this->draw_vram);
  if (this->mode == 2) {
    for (int i = 0; i < this->get_father()->p_sheet.get_size(); i++) {
      if (this->get_father()->p_sheet[i] == this) {
        this->get_father()->p_sheet[i] = nullptr;
        break;
      }
    }
    this->get_father()->refresh(vx0, vy0, this->get_xsize() + vx0, this->get_ysize() + vy0);
  }
}
void PSheetBase::slide(int sx, int sy) {
  if (mode == 2) {
    printk("slide %d %d\n", sx, sy);
    int old_x = this->now_x, old_y = this->now_y;
    sheet_slide(this->sht, sx, sy);
    this->now_x = sx;
    this->now_y = sy;
    this->get_father()->refresh(old_x, old_y, old_x + this->x_size,
                                old_y + this->y_size);
    this->refresh(0, 0, this->x_size, this->y_size);
  } else if (mode == 1) {
    sheet_slide(this->sht, sx, sy);
    this->now_x = sx;
    this->now_y = sy;
  }
}
struct SHEET* PSheetBase::add(PSheetBase* ps, vram_t* vbuf, int col_ent) {
  struct SHEET* res = sheet_alloc(stl);  // 因为需要使用窗口内的窗口
  sheet_setbuf(res, vbuf, ps->get_xsize(), ps->get_ysize(), col_ent);
  p_sheet.push_back(ps);
  // sheet_slide(res,init_x,init_y);
  // sheet_updown(res,1);
  return res;
}
void PSheetBase::updown(int high) {
  sheet_updown(this->sht, high);
  this->refresh(0, 0, this->x_size, this->y_size);
}
PSheetBase* PSheetBase::get_father() {
  switch (mode) {
    case 1:
      return this;  // 非窗口内窗口，没有父亲
      break;
    case 2:
      return dad;  // 有父亲
      break;
    default:
      return NULL;
      break;
  }
}
struct SHEET* PSheetBase::get_sht() {
  return this->sht;
}
struct SHTCTL* PSheetBase::get_stl() {
  return this->stl;
}
void PSheetBase::refresh(int x, int y, int x1, int y1) {
  if (mode == 2) {
    sheet_refresh(this->outermost_sht, x, y, x1, y1);
    sheet_refresh(this->sht, x, y, x1, y1);
    this->get_father()->refresh(x + this->sht->vx0, y + this->sht->vy0,
                                x1 + this->sht->vx0, y1 + this->sht->vy0);
  } else if (mode == 1) {
    //   printk("call ctl.... %d %d %d %d\n", x, y, x1, y1);
    sheet_refresh(this->outermost_sht, x, y, x1, y1);
    sheet_refresh(this->sht, x, y, x1, y1);
  }
}
vram_t* PSheetBase::get_vram() {
  return this->draw_vram;
}
int PSheetBase::get_xsize() {
  return this->x_size;
}
int PSheetBase::get_ysize() {
  return this->y_size;
}

struct SHEET* get_sheet_from_xy(struct SHTCTL* ctl,
                                int x,
                                int y,
                                int* sx,
                                int* sy) {
  for (int j = ctl->top - 1; j > 0; j--) {
    struct SHEET* sht = ctl->sheets[j];
    if (x >= sht->vx0 && x <= sht->vx0 + sht->bxsize && y >= sht->vy0 &&
        y <= sht->vy0 + sht->bysize) {
      *sx = x - sht->vx0;
      *sy = y - sht->vy0;
      return sht;
    }
  }
  return (struct SHEET*)(0xffffffff);
}
PSheetBase* psheet_get_outermost(PSheetBase* sht) {
  while (sht != sht->get_father()) {
    sht = sht->get_father();
  }
}
void psheet_onclick_callback(PSheetBase* ps, int x, int y, uint32_t val) {
  for (int i = 0; i < ps->pbox.get_size(); i++) {
    PBox b = ps->pbox[i];
    int nx, nx1, ny, ny1;
    nx = b.x;
    ny = b.y;
    nx1 = b.x1;
    ny1 = b.y1;

    if (x >= nx && x <= nx1 && y >= ny && y <= ny1) {
      b.callback(b.ps, x - nx, y - ny, b.val);
      if (b.ps->get_father() != b.ps) {
        b.ps->get_father()->p_left_last = b.ps;
        b.ps->get_father()->p_right_last = nullptr;
        b.ps->get_father()->p_stay_last = nullptr;
      }
    }
  }
}
void psheet_onclick_callback_right(PSheetBase* ps, int x, int y, uint32_t val) {
  //  printk("right?\n");
  for (int i = 0; i < ps->pbox_right.get_size(); i++) {
    PBox b = ps->pbox_right[i];
    int nx, nx1, ny, ny1;
    nx = b.x;
    ny = b.y;
    nx1 = b.x1;
    ny1 = b.y1;

    if (x >= nx && x <= nx1 && y >= ny && y <= ny1) {
      b.callback(b.ps, x - nx, y - ny, b.val);
      if (b.ps->get_father() != b.ps) {
        b.ps->get_father()->p_left_last = nullptr;
        b.ps->get_father()->p_right_last = b.ps;
        b.ps->get_father()->p_stay_last = nullptr;
      }
    }
  }
}
void psheet_onclick_callback_stay(PSheetBase* ps, int x, int y, uint32_t val) {
  for (int i = 0; i < ps->pbox_stay.get_size(); i++) {
    PBox b = ps->pbox_stay[i];
    int nx, nx1, ny, ny1;
    nx = b.x;
    ny = b.y;
    nx1 = b.x1;
    ny1 = b.y1;
    if (x >= nx && x <= nx1 && y >= ny && y <= ny1) {
      b.callback(b.ps, x - nx, y - ny, b.val);
      if (b.ps->get_father() != b.ps) {
        b.ps->get_father()->p_left_last = nullptr;
        b.ps->get_father()->p_right_last = nullptr;
        b.ps->get_father()->p_stay_last = b.ps;
      }
    }
  }
}
void PSheetBase::regiser_box_left(
    int x,
    int y,
    int x1,
    int y1,
    uint32_t val,
    void (*callback)(PSheetBase* ps, int x, int y, uint32_t val)) {
  PBox b;
  b.callback = callback;
  b.ps = this;
  b.x = x;
  b.x1 = x1;
  b.y = y;
  b.y1 = y1;
  b.val = val;
  this->pbox.push_back(b);
  if (!this->reg_flag) {
    if (this->mode == 1) {
      click_left->register_box(this, 0, 0, this->get_xsize(), this->get_ysize(),
                               0, psheet_onclick_callback);
    } else if (this->mode == 2) {
      this->get_father()->regiser_box_left(
          this->get_sht()->vx0, this->get_sht()->vy0,
          this->get_sht()->vx0 + this->get_xsize(),
          this->get_sht()->vy0 + this->get_ysize(), 0, psheet_onclick_callback);
      if (!reg_flag_stay) {
        this->get_father()->regiser_box_stay(
            this->get_sht()->vx0, this->get_sht()->vy0,
            this->get_sht()->vx0 + this->get_xsize(),
            this->get_sht()->vy0 + this->get_ysize(), 0,
            psheet_onclick_callback_stay);
      }
      if (!reg_flag_right) {
        this->get_father()->regiser_box_right(
            this->get_sht()->vx0, this->get_sht()->vy0,
            this->get_sht()->vx0 + this->get_xsize(),
            this->get_sht()->vy0 + this->get_ysize(), 0,
            psheet_onclick_callback_right);
      }
      io_cli();
      this->get_father()->pbox[this->get_father()->pbox.get_size() - 1].ps =
          this;
      if (!reg_flag_stay) {
        this->get_father()
            ->pbox_stay[this->get_father()->pbox_stay.get_size() - 1]
            .ps = this;
      }
      if (!reg_flag_right) {
        this->get_father()
            ->pbox_right[this->get_father()->pbox_right.get_size() - 1]
            .ps = this;
      }
      io_sti();
      reg_flag_stay = 1;
      reg_flag_right = 1;
    }
  }
  reg_flag = 1;
}
void PSheetBase::regiser_box_right(
    int x,
    int y,
    int x1,
    int y1,
    uint32_t val,
    void (*callback)(PSheetBase* ps, int x, int y, uint32_t val)) {
  PBox b;
  b.callback = callback;
  b.ps = this;
  b.x = x;
  b.x1 = x1;
  b.y = y;
  b.y1 = y1;
  b.val = val;
  this->pbox_right.push_back(b);
  if (!this->reg_flag_right) {
    if (this->mode == 1) {
      click_right->register_box(this, 0, 0, this->get_xsize(),
                                this->get_ysize(), 0,
                                psheet_onclick_callback_right);
    } else if (this->mode == 2) {
      this->get_father()->regiser_box_right(
          this->get_sht()->vx0, this->get_sht()->vy0,
          this->get_sht()->vx0 + this->get_xsize(),
          this->get_sht()->vy0 + this->get_ysize(), 0,
          psheet_onclick_callback_right);
      if (!reg_flag_stay) {
        this->get_father()->regiser_box_stay(
            this->get_sht()->vx0, this->get_sht()->vy0,
            this->get_sht()->vx0 + this->get_xsize(),
            this->get_sht()->vy0 + this->get_ysize(), 0,
            psheet_onclick_callback_stay);
      }
      if (!reg_flag) {
        this->get_father()->regiser_box_left(
            this->get_sht()->vx0, this->get_sht()->vy0,
            this->get_sht()->vx0 + this->get_xsize(), 0,
            this->get_sht()->vy0 + this->get_ysize(), psheet_onclick_callback);
      }
      io_cli();
      this->get_father()->pbox[this->get_father()->pbox.get_size() - 1].ps =
          this;
      if (!reg_flag_stay) {
        this->get_father()
            ->pbox_stay[this->get_father()->pbox_stay.get_size() - 1]
            .ps = this;
      }
      if (!reg_flag_right) {
        this->get_father()
            ->pbox_right[this->get_father()->pbox_right.get_size() - 1]
            .ps = this;
      }
      io_sti();
      reg_flag_stay = 1;
      reg_flag_right = 1;
    }
  }
  reg_flag = 1;
}
void PSheetBase::regiser_box_stay(
    int x,
    int y,
    int x1,
    int y1,
    uint32_t val,
    void (*callback)(PSheetBase* ps, int x, int y, uint32_t val)) {
  PBox b;
  b.callback = callback;
  b.ps = this;
  b.x = x;
  b.x1 = x1;
  b.y = y;
  b.y1 = y1;
  b.val = val;
  this->pbox_stay.push_back(b);
  if (!this->reg_flag_stay) {
    // printk("Reg stay!\n");
    if (this->mode == 1) {
      stay_box->register_box(this, 0, 0, this->get_xsize(), this->get_ysize(),
                             0, psheet_onclick_callback_stay);
    } else if (this->mode == 2) {
      this->get_father()->regiser_box_stay(
          this->get_sht()->vx0, this->get_sht()->vy0,
          this->get_sht()->vx0 + this->get_xsize(),
          this->get_sht()->vy0 + this->get_ysize(), 0,
          psheet_onclick_callback_stay);
      io_cli();
      this->get_father()->pbox_stay[pbox_stay.get_size() - 1].ps = this;
      io_sti();
    }
  }
  reg_flag_stay = 1;
}
extern PSheetBase *left_last, *stay_last, *right_last;
PSheetBase* PSheetBase::get_left_last() {
  if (this->get_father() != this) {
    return this->get_father()->p_left_last;
  } else {
    return left_last;
  }
}
PSheetBase* PSheetBase::get_right_last() {
  if (this->get_father() != this) {
    return this->get_father()->p_right_last;
  } else {
    return right_last;
  }
}
