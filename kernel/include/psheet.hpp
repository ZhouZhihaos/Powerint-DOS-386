#ifndef __PSHEET__
#define __PSHEET__
extern "C" {
#include <dos.h>
}
#include <pvec.hpp>
class PSheetBase;
struct PBox {
  PSheetBase* ps;
  int x, y, x1, y1;
  uint32_t val;
  void (*callback)(PSheetBase* ps, int x, int y, uint32_t val);
};
class PSheetBase {
 public:
  PVector<PBox> pbox;
  PVector<PBox> pbox_right;
  PVector<PBox> pbox_stay;
  PSheetBase* p_left_last;
  PSheetBase* p_stay_last;
  PSheetBase* p_right_last;

  PSheetBase(struct SHTCTL* ctl,
             int xsize,
             int ysize,
             int init_x,
             int init_y,
             int col_ent);  // 创建，非窗口内窗口
  PSheetBase(PSheetBase* ps, int xsize, int ysize, int x, int y, int col_ent);
  ~PSheetBase();
  void slide(int sx, int sy);
  struct SHEET* add(PSheetBase* ps, vram_t* vbuf, int col_ent);
  void updown(int high);
  PSheetBase* get_father();
  struct SHEET* get_sht();
  struct SHTCTL* get_stl();
  vram_t* get_vram();
  void refresh(int x, int y, int x1, int y1);
  void regiser_box_left(
      int x,
      int y,
      int x1,
      int y1,
      uint32_t val,
      void (*callback)(PSheetBase* ps, int x, int y, uint32_t val));
  void regiser_box_stay(
      int x,
      int y,
      int x1,
      int y1,
      uint32_t val,
      void (*callback)(PSheetBase* ps, int x, int y, uint32_t val));
  void regiser_box_right(
      int x,
      int y,
      int x1,
      int y1,
      uint32_t val,
      void (*callback)(PSheetBase* ps, int x, int y, uint32_t val));
  PSheetBase* get_right_last();
  PSheetBase* get_left_last();
  int get_xsize();
  int get_ysize();

 protected:
  struct SHTCTL* stl;  // 只需要存自己的ctl就行了
  PVector<PSheetBase*> p_sheet;
  vram_t *vram, *draw_vram;
  struct SHEET *sht, *outermost_sht;
  PSheetBase* dad;
  int now_x, now_y;
  int x_size, y_size;
  int mode, reg_flag, reg_flag_stay, reg_flag_right;
};
struct SHEET* get_sheet_from_xy(struct SHTCTL* ctl,
                                int x,
                                int y,
                                int* sx,
                                int* sy);
#endif