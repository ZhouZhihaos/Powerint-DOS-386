#ifndef _GUI_H
#define _GUI_H
#include <define.h>
// button.c
bool Collision(int x, int y, int w, int h, int x1, int y1);
bool Button_Click(Button* buf, int _x, int _y);
void DeleteButton(Button* buf);
void Button_Draw(Button* buf);
void Button_Draw_Clicking(Button* buf);
Button* MakeButton(int x,
                   int y,
                   int w,
                   int h,
                   struct SHEET* buf,
                   char* text,
                   void (*OnClick)());
// window.c
void make_window(vram_t* buf, int xsize, int ysize, char* title);
struct SHEET* MakeWindow(int x,
                         int y,
                         int w,
                         int h,
                         char* title,
                         struct SHTCTL* stl,
                         vram_t* vbuf,
                         void (*Close)());
void make_textbox8(struct SHEET* sht, int x0, int y0, int sx, int sy, int c);
TextBox* MakeTextBox(int x, int y, int w, int h, struct SHEET* window_sht);
bool TextBox_Click(TextBox* buf, int _x, int _y);
void TextBox_AddChar(TextBox* buf, char ch);
void Draw_text_box_Cur_sht(struct SHEET* buf, int x, int y);
void Draw_Cur(vram_t* vram, int x, int y, int xsize);
void MsgBoxClose();
void DeleteTextBox(TextBox* buf);
void MsgBox(char* infomation, char* title);
// graphic.c
void init_screen(vram_t* vram, int x, int y);
void init_mouse_cursor(vram_t* mouse,int bc);
void graphic(void);
// gui_syscall.c
void Gui_API(int edi,
             int esi,
             int ebp,
             int esp,
             int ebx,
             int edx,
             int ecx,
             int eax);
// listbox.c
listBox_t* MakeListBox(struct SHEET* sheet, int x, int y, int w, int h);
void AddItem(listBox_t* listBox, char* item);
void RemoveItem(listBox_t* listBox, int index);
void DeleteListBox(listBox_t* listBox);
#endif