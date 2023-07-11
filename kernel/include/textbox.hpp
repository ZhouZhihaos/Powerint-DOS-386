#ifndef __TEXTBOX__HPP__
#define __TEXTBOX__HPP__
extern "C" {
  #include <mstr.h>
}
#include <psheet.hpp>
class PTextBox : public PSheetBase {
 public:
  int x,y;
  PTextBox(PSheetBase* ps, int x, int y, int xsize, int ysize);
  ~PTextBox();
  char *Text();
  mstr *m;
};
#endif