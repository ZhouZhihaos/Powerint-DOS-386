#include "system.h"
int x, y;
int cons_x, cons_y;
void clear()
{
	int i;
	int j;
	for (i = 0; i < 160; i += 2)
	{
		for (j = 0; j < 25; j++)
		{
			*(char *)(0xb8000 + j * 160 + i) = 0x20;
			*(char *)(0xb8000 + j * 160 + i + 1) = 0x07;
		}
	}
	x = 0;
	y = 0;
	cons_x = 0;
	cons_y = 0;
	Move_Cursor(cons_x, cons_y);
}
int getlength(const char *str)
{
	int i;
	for (i = 0;; ++i)
	{
		if (str[i] == '\0')
		{
			return i;
		}
	}
}
void putstr(const char *str, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		if (y == 24 && x >= 160)
		{
			/*暂时什么也不做*/
			//TODO:屏幕滚动！！！！
			screen_ne();
		}
		if (str[i] == 0x0d)
		{
			continue;
		}
		putchar(str[i]);
	}
}
void printchar(char ch)
{
	char ch1[] = {ch, '\0'};
	print(ch1);
}
void outputchar_b800(int x, int y, char ch)
{
	x *= 2;
	*(char *)(0xb8000 + y * 160 + x) = ch;
}
void putchar(char ch)
{

	if (x == 160) {
		y++;
		cons_y++;
		x = 0;
		cons_x = 0;
		Move_Cursor(cons_x, cons_y);
	}
	if (ch == '\n') {
		if (y == 24) {
			screen_ne();
			return;
		}
		y++;
		cons_y++;
		x = 0;
		cons_x = 0;
		Move_Cursor(cons_x, cons_y);
		return;
	} else if (ch == '\0') {
		return;
	} else if (ch == '\b') {
		if (x == 0) {
			cons_y--;
			y--;
			cons_x=79;
			x=158;
			Move_Cursor(cons_x,cons_y);
			*(char *)(0xb8000 + y * 160 + x) = ' ';
			return;
		}
		*(char *)(0xb8000 + y * 160 + x - 2) = ' '; /*显示位置为第23行*/
		x -= 2;
		cons_x--;
		Move_Cursor(cons_x, cons_y);
		return;
	}
	cons_x += 1;
	Move_Cursor(cons_x, cons_y);
	*(char *)(0xb8000 + y * 160 + x) = ch; /*显示位置为第23行*/
	x += 2;
}
void Move_Cursor(short x, short y)
{
	int res = y * 80 + x;
	ASM_call(res);
}
void print(const char *str)
{
	putstr(str, getlength(str));
}
void screen_ne() /*向下滚动一行*/
{
	//TODO:屏幕滚动！！！！
	int i;
	int j;
	for (i = 0; i < 160; i += 2)
	{
		for (j = 0; j < 24; j++)
		{
			*(char *)(0xb8000 + j * 160 + i) = *(char *)(0xb8000 + (j + 1) * 160 + i);
			*(char *)(0xb8000 + j * 160 + i + 1) = *(char *)(0xb8000 + (j + 1) * 160 + i + 1);
		}
	}
	for (i = 0; i < 160; i += 2)
	{
		*(char *)(0xb8000 + 24 * 160 + i) = ' ';
		*(char *)(0xb8000 + 24 * 160 + i + 1) = 0x07;
	}

	cons_x = 0;
	x = 0;
	Move_Cursor(cons_x, cons_y);
}
int get_x(){
	return cons_x;
}
int get_y(){
	return cons_y;
}
void printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	char buf[1024];
	vsprintf(buf, format, ap);
	print(buf);
	va_end(ap);
}
void gotoxy(int x1, int y1)
{
	cons_x = x1;
	cons_y = y1;
	x = x1*2;
	y = y1;
	Move_Cursor(cons_x, cons_y);
}
