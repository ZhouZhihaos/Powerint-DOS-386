// Powerint DOS 386系统调用
// @ Copyright (C) 2022
// @ Author: zhouzhihao & min0911_
#ifndef SYS_CALL_H
#define SYS_CALL_H
#include <ctypes.h>
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
int Get_System_Version();
void Copy(char *filePath1, char *filePath2);
int _kbhit();
void mkfile(char *filename);
void mkdir(char *filename);
void Edit_File(char *filename,char *dest,int len);
void SwitchTo320X200X256_BIOS();
void SwitchToText8025_BIOS();
int AddTask(char *taskname,int eip);
void SubTask(int id);
void TaskForever();
void SendMessage(int to_tid, void *data, unsigned int size);
void GetMessage(void *data, int from_tid);
unsigned int MessageLength(int from_tid);
int NowTaskID();
void SwitchToHighTextMode();
void HighPrint(char *str);
void ReTurnToNornalTextMode();
void exit();
void timer_alloc();
void timer_settime(unsigned int time);
int timer_out();
void timer_free();
int haveMsg();
void GetMessageAll(void *data);
char PhyMemGetByte(int addr);
void PhyMemSetByte(int addr, char data);
#endif