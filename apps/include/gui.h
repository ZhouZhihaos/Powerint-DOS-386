#ifndef __GUI_H__
#define __GUI_H__
#ifdef __cplusplus
extern "C" {
#endif
#define BUTTON_MSG_ID -1
#define WINDOW_CLOSE_MSG_ID -2
typedef int window_t;
typedef int button_t;
typedef unsigned int vram_t;
typedef vram_t color_t;
typedef int HWND; // 句柄
window_t create_window(int x, int y, int w, int h, char* title);
button_t create_button(int x, int y, int w, int h, window_t wid, char* title);
void close_window(window_t wid);
void delete_button(button_t bid);
void SDraw_Px(window_t wid, int x, int y, color_t color, int xsize);
void SDraw_Box(window_t wid,
               int x,
               int y,
               int x1,
               int y1,
               color_t color,
               int xsize);
void SDraw_Char(window_t wid, int x, int y, int c, color_t color, int xsize);
void Sputs(window_t wid, char* str, int x, int y, color_t col, int xsize);
void PrintChineseChar0(window_t wid,
                       int xsize,
                       int x,
                       int y,
                       color_t color,
                       unsigned short cChar);
void PrintChineseStr0(window_t wid,
                      int xsize,
                      int x,
                      int y,
                      color_t color,
                      unsigned char* str);
void MsgBox(char* infomation, char* title);
int IsGuiMode(void);
HWND MakeTextBox(int x, int y, int w, int h, window_t window_sht);
void DeleteTextBox(HWND hwnd);
void GetTextBoxText(char *result, HWND hwnd);
HWND MakeListBox(window_t wid, int x, int y, int w, int h);
void AddItem(HWND* listBox, char* item);
void RemoveItem(HWND* listBox, int index);
void DeleteListBox(HWND* listBox);
#ifdef __cplusplus
}
#endif
#endif