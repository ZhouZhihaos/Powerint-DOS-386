// Graphic.c : 图形界面的简单实现
// 依赖：kernel.c shell.c mouse.c mem.c sheet.c vbe.c vga.c
// Copyright(C) 2022 zhouzhihao & min0911_
//-------------------------------
#include "system.h"
extern unsigned int memsize;
char *font;
void init_screen8(char *vram, int x, int y)
{
	memset(vram, 14, x * y);
}
void PUTCHINESE0(unsigned char *vram, int x, int y, char color, unsigned short CH, int xsize)
{
	extern char *hzkfont;
	int i, j, k, offset;
	int flag;
	unsigned char buffer[32];
	unsigned char word[2] = {CH & 0xff, (CH & 0xff00) >> 8}; // 将字符转换为两个字节
	unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	unsigned char *p = hzkfont;
	offset = (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
	p = p + offset;
	//读取，并写入到vram中
	for (i = 0; i < 32; i++)
	{
		buffer[i] = p[i];
	}
	for (k = 0; k < 16; k++)
	{
		for (j = 0; j < 2; j++)
		{
			for (i = 0; i < 8; i++)
			{
				flag = buffer[k * 2 + j] & key[i];
				if (flag)
				{
					// Draw_Px(x + i + j * 8, y + k, color);
					vram[(y + k) * xsize + (x + i + j * 8)] = color;
				}
			}
		}
	}
}
void init_mouse_cursor8(char *mouse, char bc)
/* 鼠标的数据准备（16x16） */
{
	int x, y;

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			if (mouse_cur_graphic[y][x] == '*')
			{
				mouse[y * 16 + x] = 0x0;
			}
			if (mouse_cur_graphic[y][x] == 'O')
			{
				mouse[y * 16 + x] = 0xf;
			}
			if (mouse_cur_graphic[y][x] == '.')
			{
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}
void Sputs(unsigned char *vram, char *str, int x, int y, int col, int xsize)
{
	int i;
	for (i = 0; str[i] != 0; i++)
	{
		SDraw_Char(vram, x + i, y, str[i], col, xsize);
	}
}
void PowerMakeWindow(unsigned char *vram, int xsize, int ysize)
{
	// 160*52
	for (int i = 0; i < ysize; i++)
	{
		for (int j = 0; j < xsize; j++)
		{
			vram[i * xsize + j] = 0xf;
		}
	}
}
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++)
	{
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void SDraw_Px(unsigned char *vram, int x, int y, int color, int xsize)
{
	unsigned char *p;
	p = vram + (y * xsize + x);
	*p = color;
	return;
}
void SDraw_Box(unsigned char *vram, int x, int y, int x1, int y1, int color, int xsize)
{
	int i, j;
	for (i = x; i <= x1; i++)
	{
		for (j = y; j <= y1; j++)
		{
			SDraw_Px(vram, i, j, color, xsize);
		}
	}
	return;
}
void SDraw_Char(unsigned char *vram1, int x, int y, char c, int color, int xsize)
{
	extern char *ascfont;
	x *= 8;
	y *= 16;
	unsigned char *vram = (unsigned char *)vram1;
	unsigned char *p;
	unsigned char *font;
	int i, j;
	font = ascfont;
	font += c * 16;
	int char_color = color & 0x0f;
	int bg_color = (color & 0xf0) >> 4;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (font[i] & (0x80 >> j))
			{
				p = vram + (y + i) * xsize + x + j;
				*p = color;
			}
		}
	}
	return;
}
int sc2a(int sc)
{
	int ch = sc;
	if (keytable[ch] == 0x00)
	{
		return 0;
	}
	if (ch == 0x48 || ch == 0x50 || ch == 0x4b || ch == 0x4d) //  Ƿ         ַ
	{
		if (ch == 0x48)
		{
			return -1;
		}
		else if (ch == 0x50)
		{
			return -2;
		}
		else if (ch == 0x4b)
		{
			return -3;
		}
		else if (ch == 0x4d)
		{
			return -4;
		}
	}
	else if (shift == 0 && caps_lock == 0)
	{
		return keytable1[ch];
	}
	else if (shift == 1 || caps_lock == 1)
	{
		return keytable[ch];
	}
	else if (shift == 1 && caps_lock == 1)
	{
		return keytable1[ch];
	}
}
int first1 = 0;
void Draw_Cur(unsigned char *vram, int x, int y, int xsize)
{
	static char CUR[16][8] = {
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"........",
		"********",
		"********"};
	int i, j;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (CUR[i][j] == '.')
			{
				vram[(y + i) * xsize + x + j] = 99;
			}
			else if (CUR[i][j] == '*')
			{
				vram[(y + i) * xsize + x + j] = 0xf;
			}
		}
	}
}
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"};

	int x, y;
	char c;
	boxfill8(buf, xsize, 0x7, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, 0xf, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, 0x7, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, 0xf, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, 0x8, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, 0x0, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, 0x7, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, 12, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, 0x8, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, 0x0, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, 0xf, title);

	for (y = 0; y < 14; y++)
	{
		for (x = 0; x < 16; x++)
		{
			c = closebtn[y][x];
			if (c == '@')
			{
				c = 0x0;
			}
			else if (c == '$')
			{
				c = 8;
			}
			else if (c == 'Q')
			{
				c = 7;
			}
			else
			{
				c = 0xf;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}
unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cur, *buf_win2;
struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cur, *sht_win2;
void window2()
{
	int c = 0;
	while (1)
	{
		char s[512];
		sprintf(s, "%d", c);
		putfonts8_asc_sht(sht_win2, 8, 28, 0x0, 0x7, s, strlen(s));
		c++;
	}
}
void graphic(void)
{
	
	int i, c1 = 0;
	int x, y;
	int c = 0;
	char s1[512];
	char s[512];
	struct SHTCTL *shtctl;
	struct FIFO32 fifo;
	int fifobuf[128];
	int gmx, gmy;
	clear();
	struct VBEINFO *vbinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
	SwitchVBEMode(MODE_1024X768X256);
	init_palette();
	// memset(0xa0000, 14, 0x10000);
	fifo32_init(&fifo, 128, fifobuf);
	struct TIMER *timer;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 25);
	shtctl = shtctl_init(vbinfo->vram, vbinfo->xsize, vbinfo->ysize);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	sht_cur = sheet_alloc(shtctl);
	sht_win2 = sheet_alloc(shtctl);
	buf_back = (unsigned char *)page_kmalloc(vbinfo->xsize * vbinfo->ysize);
	buf_win = (unsigned char *)page_kmalloc(160 * 104);
	buf_cur = (unsigned char *)page_kmalloc(135 * 75);
	buf_win2 = (unsigned char *)page_kmalloc(135 * 75);
	// PowerMakeWindow(buf_back,80,25);
	sheet_setbuf(sht_back, buf_back, vbinfo->xsize, vbinfo->ysize, -1); /* 没有透明色 */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);						/* 透明色号99 */
	sheet_setbuf(sht_win, buf_win, 135, 75, -1);
	sheet_setbuf(sht_cur, buf_cur, 8, 16, 99);
	sheet_setbuf(sht_win2, buf_win2, 135, 75, -1);
	init_screen8(buf_back, vbinfo->xsize, vbinfo->ysize);
	Draw_Cur(buf_cur, 0, 0, 8);
	init_mouse_cursor8(buf_mouse, 99); /* 背景色号99 */
	make_window8(buf_win, 135, 75, "Console");
	make_window8(buf_win2, 135, 75, "window2");
	boxfill8(buf_win, 135, 0, 5, 24, 130, 70);
	sheet_slide(sht_back, 0, 0);
	gmx = (vbinfo->xsize - 16) / 2; /* 按显示在画面中央来计算坐标 */
	gmy = (vbinfo->ysize - 28 - 16) / 2;
	sheet_slide(sht_mouse, gmx, gmy);
	sheet_slide(sht_win, 180, 172);
	sheet_slide(sht_cur, 0, 0);
	sheet_slide(sht_win2, 200, 300);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);
	sheet_updown(sht_cur, 3);
	sheet_updown(sht_win2, 1);
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 0, 0xf, "Power Desktop V0.1");
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 1 * 16, 0xf, "Powered by:");
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 2 * 16, 0xf, "Powerint DOS 386");
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 3 * 16, 0xf, "KernelVersion:0.5c");
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 4 * 16, 0xf, "Copyright(C) 2022");
	SDraw_Box(buf_back, 0, vbinfo->ysize - 16, 8 * 8, vbinfo->ysize, 0, vbinfo->xsize);
	putfonts8_asc(buf_back, vbinfo->xsize, 0, vbinfo->ysize - 16, 0xf, "DOS MODE");

	sprintf(s, "memtotal:%dMB", memsize / (1024 * 1024));
	putfonts8_asc(buf_back, vbinfo->xsize, 0, 6 * 16, 0xf, s);
	sheet_refresh(sht_back, 0, 0, vbinfo->xsize, vbinfo->ysize); /* 刷新文字 */
	// printf("Power Desk TopV0.1 我们有了图形用户操作界面！\n");
	char *window2_esp = page_kmalloc(1024*1024);
	struct TASK *Task_Window2 = AddTask("Window2", 3, 2 * 8, (int)window2, 1 * 8, 1 * 8, (int)window2_esp);
	SleepTaskFIFO(Task_Window2);	// window2记数 不需要fifo RIGHT?
	int cx = 8;
	sheet_slide(sht_cur, 180 + cx, 172 + 28);
	mouse_ready(&mdec);
	for (;;)
	{
		// printf("ABC");
		if (fifo8_status(TaskGetMousefifo(NowTask())) + fifo8_status(TaskGetKeyfifo(NowTask())) + fifo32_status(&fifo) == 0)
		{
			// putfonts8_asc(buf_back, 320, 0, 4 * 16, 0xf, "Copyright(C) 2022");
		}
		else
		{
			if (fifo8_status(TaskGetMousefifo(NowTask())) != 0)
			{
				i = fifo8_get(TaskGetMousefifo(NowTask()));
				// fifo8_get(TaskGetKeyfifo(NowTask()));
				//  io_sti();
				if (mouse_decode(&mdec, i) != 0)
				{
					gmx += mdec.x;
					gmy += mdec.y;
					if (gmx > vbinfo->xsize - 10)
					{
						gmx = vbinfo->xsize - 10;
					}
					else if (gmx < 0)
					{
						gmx = 0;
					}
					if (gmy > vbinfo->ysize - 16)
					{
						gmy = vbinfo->ysize - 16;
					}
					else if (gmy < 0)
					{
						gmy = 0;
					}
					if ((mdec.btn & 0x01) != 0)
					{
						// back to dos mode
						if (gmy >= vbinfo->ysize - 16 && gmy <= vbinfo->ysize && gmx >= 0 && gmx <= 8 * 8)
						{
							SubTask(Task_Window2);
							mouse_sleep(&mdec);
							page_kfree((int)buf_back, vbinfo->xsize * vbinfo->ysize);
							page_kfree((int)buf_mouse, 16 * 16);
							page_kfree((int)buf_win, 160 * 104);
							page_kfree((int)buf_win2, 135 * 75);
							page_kfree((int)buf_cur, 135 * 75);
							page_kfree((int)window2_esp, 1024*1024);
							sheet_free(sht_back);
							sheet_free(sht_mouse);
							sheet_free(sht_win);
							sheet_free(sht_cur);
							sheet_free(sht_win2);
							timer_free(timer);
							ctl_free(shtctl);
							SwitchToText8025_BIOS(); //调用BIOS功能
							Set_Font("A:\\other\\font.bin");
							return;

						}
						else
						{
							sheet_slide(sht_win, gmx - 80, gmy - 8);
							sheet_slide(sht_cur, sht_win->vx0 + cx, sht_win->vy0 + 28);
						}
						// Sputs(buf_back, "Left", 0, 0, 0xf, 320);
						// sheet_refresh(sht_back, 0, 0, 320, 100);
					}
					else if ((mdec.btn & 0x02) != 0)
					{
						// Sputs(buf_back, "Right", 0, 0, 0xf, 320);
						// sheet_refresh(sht_back, 0, 0, 320, 100);
					}
					else if ((mdec.btn & 0x04) != 0)
					{
						// Sputs(buf_back, "MIDDLE", 0, 0, 0xf, 320);
						// sheet_refresh(sht_back, 0, 0, 320, 100);
					}
					sheet_slide(sht_mouse, gmx, gmy);
				}
			}
			if (fifo8_status(TaskGetKeyfifo(NowTask())) != 0)
			{
				i = fifo8_get(TaskGetKeyfifo(NowTask()));
				if (i > 0x80)
				{
					continue;
				}
				i = sc2a(i);
				// io_sti();
				if (i == '\b')
				{
					if (cx > 8)
					{
						cx -= 8;
						putfonts8_asc_sht(sht_win, cx, 28, 0xf, 0, " ", 1);
					}
				}
				else if (cx <= 14 * 8 && i != 0)
				{
					sprintf(s, "%c", i);
					putfonts8_asc_sht(sht_win, cx, 28, 0xf, 0, s, 1);
					cx += 8;
				}
				sheet_slide(sht_cur, sht_win->vx0 + cx, sht_win->vy0 + 28);
			}
			if (fifo32_status(&fifo) != 0)
			{
				i = fifo32_get(&fifo);
				if (i == 1)
				{
					if (c1 == 0)
					{
						c1 = 1;
						sheet_updown(sht_cur, -1);
					}
					else
					{
						c1 = 0;
						sheet_updown(sht_cur, 3);
					}
					timer_settime(timer, 25);
				}
			}
		}
	}
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}
