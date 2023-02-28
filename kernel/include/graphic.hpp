#ifndef __GRA__HPP__
#define __GRA__HPP__
extern "C" {
#include <dos.h>
}
namespace ui {
GuiPosition POSITION(int x, int y, int w, int h);
class PowerWindow {
 public:
  PowerWindow(const GuiPosition& pos, char* title, void (*callback)(void));
  ~PowerWindow();
  GuiPosition Pos();
  char* Title();
  struct SHEET* Sheet();
  void Line(int x1, int y1, int x2, int y2, color_t color);
  void Box(int x1, int y1, int x2, int y2, color_t color);
  void Char(int x, int y, char c, color_t color);
  void Text(int x, int y, char* text, color_t color);
  void Px(int x, int y, color_t color);
  void Clear(color_t color);
  vram_t* Vram();

 private:
  struct SHEET* sheet;
  char* title;
  GuiPosition pos;
  vram_t* vram;
};
class PowerButton {
 public:
  PowerButton(PowerWindow* dad,
              const GuiPosition& pos,
              char* text,
              void (*callback)(void));
  ~PowerButton();
  GuiPosition Pos();
  char* Text();

 private:
  GuiPosition pos;
  char* text;
  Button* button;
  PowerWindow* dad;
};

class PowerTextBox {
 public:
  PowerTextBox(PowerWindow* dad, const GuiPosition& pos);
  ~PowerTextBox();
  GuiPosition Pos();
  char* Text();

 private:
  GuiPosition pos;
  TextBox* textbox;
  PowerWindow* dad;
  char* text;
};

class PowerListBox {
 public:
  PowerListBox(PowerWindow* dad, const GuiPosition& pos);
  ~PowerListBox();
  GuiPosition Pos();
  char* ItemOf(int index);
  int Count();
  void Add(char* text);
  void Remove(int index);
  void Clear();

 private:
  GuiPosition pos;
  listBox_t* listbox;
  PowerWindow* dad;
  char* text;
};
}  // namespace ui
#endif