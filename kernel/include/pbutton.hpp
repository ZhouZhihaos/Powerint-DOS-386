#ifndef __BUTTON_HPP__
#define __BUTTON_HPP__
#include <psheet.hpp>
class PButton : public PSheetBase {
 public:
  PButton(PSheetBase* ps,
          char* name,
          int x,
          int y,
          int xsize,
          int ysize,
          uint32_t val,
          void (*callback)(PButton* pbtn, uint32_t val));
  ~PButton();
  void draw(char* name, color_t bc);
  int flag;
  char* name;
  uint32_t val;
  void (*callback)(PButton* pbtn, uint32_t val);
};
#endif