extern "C" {
#include <dos.h>
void boxfill(vram_t* vram,
             int xsize,
             color_t c,
             int x0,
             int y0,
             int x1,
             int y1);
}
extern struct SHTCTL* shtctl;
#include <graphic.hpp>
using namespace ui;
GuiPosition ui::POSITION(int x, int y, int w, int h) {
  GuiPosition pos;
  pos.x = x;
  pos.y = y;
  pos.w = w;
  pos.h = h;
  return pos;
}
void __null__() {}
PowerWindow::PowerWindow(const GuiPosition& pos,
                         char* title,
                         void (*callback)(void)) {
  this->pos.x = pos.x;
  this->pos.y = pos.y;
  this->pos.w = pos.w;
  this->pos.h = pos.h;

  this->title = new char[strlen(title) + 1];
  strcpy(this->title, title);
  this->vram = (vram_t*)malloc(pos.w * pos.h * sizeof(color_t));
  this->sheet = MakeWindow(this->pos.x, this->pos.y, this->pos.w, this->pos.h,
                           this->title, shtctl, this->vram, callback);
}
PowerWindow::~PowerWindow() {
  delete[] this->title;
  free(this->vram);
  sheet_free(this->sheet);
}
vram_t* PowerWindow::Vram() {
  return this->vram;
}
GuiPosition PowerWindow::Pos() {
  return this->pos;
}
char* PowerWindow::Title() {
  return this->title;
}

struct SHEET* PowerWindow::Sheet() {
  return this->sheet;
}
void PowerWindow::Line(int x1, int y1, int x2, int y2, color_t color) {
  Draw_Line(this->sheet->buf, x1, y1, x2, y2, color, this->sheet->bxsize);
  sheet_refresh(this->sheet, x1, y1, x2, y2);
}
void PowerWindow::Box(int x1, int y1, int x2, int y2, color_t color) {
  boxfill(this->sheet->buf, this->sheet->bxsize, color, x1, y1, x2, y2);
  sheet_refresh(this->sheet, x1, y1, x2, y2);
}
void PowerWindow::Char(int x, int y, char c, color_t color) {
  SDraw_Char(this->sheet->buf, x, y, c, color, this->sheet->bxsize);
  sheet_refresh(this->sheet, x, y, x + 8, y + 16);
}
void PowerWindow::Text(int x, int y, char* text, color_t color) {
  putfonts_asc(this->sheet->buf,this->sheet->bxsize, x, y, color, (unsigned char *)text);
  sheet_refresh(this->sheet, x, y, x + strlen(text) * 8, y + 16);
}
void PowerWindow::Px(int x, int y, color_t color) {
  SDraw_Px(this->sheet->buf, x, y, color, this->sheet->bxsize);
  sheet_refresh(this->sheet, x, y, x + 1, y + 1);
}
void PowerWindow::Clear(color_t color) {
  this->Box(5, 28, this->sheet->bxsize - 5, this->sheet->bysize - 5, color);
}
PowerButton::PowerButton(PowerWindow* dad,
                         const GuiPosition& pos,
                         char* text,
                         void (*callback)(void)) {
  this->dad = dad;
  this->pos.x = pos.x;
  this->pos.y = pos.y;
  this->pos.w = pos.w;
  this->pos.h = pos.h;
  this->text = new char[strlen(text) + 1];
  strcpy(this->text, text);

  this->button = MakeButton(this->pos.x, this->pos.y, this->pos.w, this->pos.h,
                            this->dad->Sheet(), this->text, callback);
}
PowerButton::~PowerButton() {
  delete[] this->text;
  DeleteButton(this->button);
}
char* PowerButton::Text() {
  return this->text;
}

PowerTextBox::PowerTextBox(PowerWindow* dad, const GuiPosition& pos) {
  this->dad = dad;
  this->pos.x = pos.x;
  this->pos.y = pos.y;
  this->pos.w = pos.w;
  this->pos.h = pos.h;
  this->dad = dad;
  this->text = (char*)NULL;
  this->textbox = MakeTextBox(this->pos.x, this->pos.y, this->pos.w,
                              this->pos.h, this->dad->Sheet());
}
PowerTextBox::~PowerTextBox() {
  if (text) {
    delete[] text;
  }
  DeleteTextBox(this->textbox);
}
char* PowerTextBox::Text() {
  if (text) {
    delete[] text;
  }
  this->text = new char[strlen(this->textbox->text) + 1];
  strcpy(this->text, this->textbox->text);
  return this->text;
}
GuiPosition PowerTextBox::Pos() {
  return this->pos;
}

PowerListBox::PowerListBox(PowerWindow* dad, const GuiPosition& pos) {
  this->pos.h = pos.h;
  this->pos.w = pos.w;
  this->pos.x = pos.x;
  this->pos.y = pos.y;
  this->dad = dad;
  this->text = (char*)NULL;
  this->listbox = MakeListBox(dad->Sheet(), this->pos.x, this->pos.y,
                              this->pos.w, this->pos.h);
}
PowerListBox::~PowerListBox() {
  if (text) {
    delete[] text;
  }
  DeleteListBox(this->listbox);
}
GuiPosition PowerListBox::Pos() {
  return this->pos;
}
void PowerListBox::Add(char* text) {
  AddItem(this->listbox, text, (uint32_t)__null__);
}
void PowerListBox::Remove(int index) {
  RemoveItem(this->listbox, index);
}
int PowerListBox::Count() {
  return this->listbox->item_num;
}
char* PowerListBox::ItemOf(int index) {
  if (text) {
    delete[] text;
  }
  this->text = new char[strlen(this->listbox->item[index]) + 1];
  strcpy(this->text, this->listbox->item[index]);
  return this->text;
}
void PowerListBox::Clear() {
  for (int i = 0; i < this->Count(); i++) {
    this->Remove(0);
  }
}
