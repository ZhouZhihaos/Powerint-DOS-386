// Powerint DOS 386系统调用
// @ Copyright (C) 2022
// @ Author: zhouzhihao & min0911_
#ifndef SYS_CALL_H
#define SYS_CALL_H
void putch(char ch);
char getch();
char input_char_inSM();
int get_xy();
int Text_get_mouse();
void goto_xy(int x, int y);
void SwitchTo320X200X256();
void SwitchToText8025();
void Draw_Char(int x, int y, char ch, int color);
void Draw_Str(int x, int y, char *str, int color);
void sleep(int time);
void PrintChineseChar(int x, int y, int color, unsigned short cChar);
void PrintChineseStr(int x, int y, int color, unsigned char *cStr);
void print(char *str);
void scan(char *str, int length);
char *malloc(int size);
void free(void *ptr, int size);
void system(char *command);
int filesize(char *filename);
int fopen(char *filename, char *res);
void bmpview(char *filename);
void Draw_Box(int x, int y, int w, int h, int color);
void Draw_Px(int x, int y, int color);
void Text_Draw_Box(int x, int y, int w, int h, int color);
int Grap_get_mouse();
void beep(int point, int notes, int dup);
void GetCmdline(char *_Str);
void drivers_idehdd_read(int lba,int sectors,char *data);
void drivers_idehdd_write(int lba,int sectors,char *data);
#endif