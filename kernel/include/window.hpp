#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__
#include <psheet.hpp>
class Window : public PSheetBase {
 public:
  void draw(char* title);
  Window(struct SHTCTL* ctl, char* title, int x, int y, int xsize, int ysize);
  Window(struct SHTCTL* ctl,
         char* title,
         int x,
         int y,
         int xsize,
         int ysize,
         void (*close)(PSheetBase* ps, int x, int y, uint32_t val));
};
#endif