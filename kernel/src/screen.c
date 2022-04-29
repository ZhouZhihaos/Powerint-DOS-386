#include "system.h"
int x, y;
int cons_x, cons_y;
unsigned int s_ne_t;
int Raw_x = 0;
int Raw_y = 0;
void clear()
{
	int i;
	int j;
	for (i = 0; i < 160; i += 2)
	{
		for (j = 0; j < 25; j++)
		{
			*(char *)(0xb8000 + j * 160 + i) = ' ';
			*(char *)(0xb8000 + j * 160 + i + 1) = get_cons_color();
		}
	}
	x = 0;
	y = 0;
	cons_x = 0;
	cons_y = 0;
	Raw_x = 0;
	Raw_y = 0;
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
	io_cli();
	int i;
	for (i = 0; i < length; i++)
	{
		if (y == 24 && x >= 160)
		{
			/*暂时什么也不做*/
			// TODO:屏幕滚动！！！！
			screen_ne();
		}
		if (str[i] == 0x0d)
		{
			continue;
		}
		putchar(str[i]);
		io_cli();
	}
	io_sti();
}
void init_s_ne_t()
{
	s_ne_t = 0;
}
int Get_S_NE_T()
{
	return s_ne_t;
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
	io_cli();
	if (x == 160)
	{
		y++;
		Raw_y++;
		cons_y++;
		x = 0;
		cons_x = 0;
		Raw_x = 0;

		Move_Cursor(cons_x, cons_y);
	}
	if (ch == '\n')
	{
		if (y == 24)
		{
			screen_ne();
			io_sti();
			return;
		}
		y++;
		cons_y++;
		x = 0;
		Raw_y++;
		Raw_x = 0;
		cons_x = 0;
		Move_Cursor(cons_x, cons_y);
		io_sti();
		return;
	}
	else if (ch == '\0')
	{
		// gotoxy(get_x()+1,get_y());
		io_sti();
		return;
	}
	else if (ch == '\b')
	{
		if (x == 0)
		{
			cons_y--;
			y--;
			cons_x = 79;
			Raw_y--;
			Raw_x = 158;
			x = 158;
			Move_Cursor(cons_x, cons_y);
			*(char *)(0xb8000 + y * 160 + x) = ' ';
			io_sti();
			return;
		}
		*(char *)(0xb8000 + y * 160 + x - 2) = ' '; /*显示位置为第23行*/
		x -= 2;
		cons_x--;
		Move_Cursor(cons_x, cons_y);
		io_sti();
		return;
	}
	else if (ch == '\t')
	{
		//制表符
		print("    ");
		io_sti();
		return;
	}
	cons_x += 1;
	Move_Cursor(cons_x, cons_y);
	*(char *)(0xb8000 + y * 160 + x) = ch; /*显示位置为第23行*/
	Raw_x += 2;

	x += 2;
	io_sti();
}
void Move_Cursor(short x, short y)
{

	int res = y * 80 + x;
	ASM_call(res);
}

void printOld(const char *str)
{
	putstr(str, getlength(str));
}
void screen_ne() /*向下滚动一行*/
{
	// TODO:屏幕滚动！！！！
	int i;
	int j;
	int X = get_x();
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
		*(char *)(0xb8000 + 24 * 160 + i + 1) = get_cons_color();
	}

	cons_x = 0;
	x = 0;
	s_ne_t++;
	Raw_y++;
	Raw_x = 0;
	Move_Cursor(cons_x, cons_y);
}
int get_x()
{
	return cons_x;
}
int get_y()
{
	return cons_y;
}
int Get_Raw_x()
{
	return Raw_x;
}
int Get_Raw_y()
{
	return Raw_y;
}
void print(const char *str)
{
	if (get_fb_seg() == 0xB800)
	{
		putstr(str, getlength(str));
	}
	else if (get_fb_seg() == 0xA000)
	{
		printf(str);
	}
}
void printf(const char *format, ...)
{
	io_cli();
	if (get_fb_seg() == 0xB800) // Text Mode
	{
		va_list ap;
		va_start(ap, format);
		char buf[1024];
		vsprintf(buf, format, ap);
		printOld(buf);
		va_end(ap);
	}
	else if (get_fb_seg() == 0xA000) // Garbage Mode
	{
		va_list ap;
		va_start(ap, format);
		char buf[1024];
		vsprintf(buf, format, ap);
		Gprint(buf);
		va_end(ap);
	}
	io_sti();
}
void GotoXy_No_Safe(int x1, int y1)
{
	cons_x = x1;
	cons_y = y1;
	x = x1 * 2;
	y = y1;
	Move_Cursor(cons_x, cons_y);
}
void gotoxy(int x1, int y1)
{
	io_cli();
	if (x1 >= 0 && y1 >= 0)
	{
		int x2 = x1;
		int y2 = y1;
		if (x1 <= 79 && y1 <= 24)
		{
			GotoXy_No_Safe(x1, y1);
			io_sti();
			return;
		}
		if (x1 <= 79)
		{
			for (int i = 0; i < y1 - 24 - 1; i++)
			{
				screen_ne();
			}
			GotoXy_No_Safe(x1, 24);
			io_sti();
			return;
		}

		if (x1 > 79)
		{
			y2 += x1 / 80 - 1;
			x2 = x1 % 80;
			if (y2 <= 24)
				gotoxy(x2, y2 + 1);
			else
				gotoxy(x2, y2);
		}
	}
	else
	{ //负数
		//如果x1为负数 说明y1要--
		if (x1 < 0)
		{
			x1 += 80;
			y1--;
			gotoxy(x1, y1);
		}
		if (y1 < 0)
		{
			io_sti();
			return;
		}
	}
	io_sti();
}
static struct VBEINFO *vinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
static unsigned char *scr_buf;
static unsigned char *cur_buf;
static struct SHEET *sht_scr, *sht_cur;
static struct SHTCTL *shtctl0;
static struct FIFO32 fifo;
static int cur_x = 0, cur_y = 0;
char *ascfont;
char *hzkfont;
void put_sht(struct SHEET *sht, int x, int y, int l)
{
	// boxfill8(sht->buf, sht->bxsize, 0, x, y, x + l * 8 - 1, y + 15);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}
void put_box(struct SHEET *sht, int x, int y, int l, int b) //覆盖文字
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	return;
}
void PutChineseChar0(unsigned char *vram, int xsize, int x, int y, char color, unsigned short cChar)
{
	PUTCHINESE0(vram, x * 16, y * 16, color, cChar, xsize);
}
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;

	for (i = 0; i < 16; i++)
	{
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0)
		{
			p[0] = c;
		}
		if ((d & 0x40) != 0)
		{
			p[1] = c;
		}
		if ((d & 0x20) != 0)
		{
			p[2] = c;
		}
		if ((d & 0x10) != 0)
		{
			p[3] = c;
		}
		if ((d & 0x08) != 0)
		{
			p[4] = c;
		}
		if ((d & 0x04) != 0)
		{
			p[5] = c;
		}
		if ((d & 0x02) != 0)
		{
			p[6] = c;
		}
		if ((d & 0x01) != 0)
		{
			p[7] = c;
		}
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	char *hankaku = ascfont;
	/* C语言中，字符串都是以0x00结尾 */
	for (; *s != 0x00; s++)
	{

		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}
void SDraw_CHAR(unsigned char *vram1, int x, int y, char c, int color, int xsize)
{
	x *= 8;
	y *= 16;
	char s[2] = {c, 0};
	putfonts8_asc(vram1, xsize, x, y, color, s);
}
static int bx = 0, by = 0;
void High_clear(void)
{
	//高分辨率模式的清屏
	memset(scr_buf, 0, vinfo->xsize * vinfo->ysize);
	sheet_refresh(sht_scr, 0, 0, vinfo->xsize, vinfo->ysize);
	bx = 0;
	by = 0;
	cur_x = 0;
	cur_y = 0;
}
void High_Screen_ne(void)
{
	//高分辨率模式下的屏幕滚动(向下,每次移动一行(8*16))
	cur_x = 0;
	cur_y = vinfo->ysize - 16;
	memcpy(scr_buf, scr_buf + vinfo->xsize * 16, vinfo->xsize * (vinfo->ysize - 16));
	memset(scr_buf + vinfo->xsize * (vinfo->ysize - 16), 0, vinfo->xsize * 16);
	sheet_refresh(sht_scr, 0, 0, vinfo->xsize, vinfo->ysize);
	bx = 0;
	by = vinfo->ysize / 16 - 1;
}
void HighPrint(char *string)
{
	char *p = string;
	int i;
	int length = strlen(string);
	unsigned char cons_color = 0x0f;
	struct VBEINFO *vinfo = VBEINFO_ADDRESS;
	if (vinfo->vram == 0)
	{
		return;
	}
	for (i = 0; i < length; i++)
	{
		// put_box(sht_scr,bx,by,1,0);
		if (p[i] == '\r')
		{
			continue;
		}
		if (by == vinfo->ysize / 16 - 1 && bx == vinfo->xsize / 8)
		{
			High_Screen_ne();
		}
		if (bx == vinfo->xsize / 8)
		{
			by++;
			Raw_y++;
			bx = 0;
			cur_x = 0;
			cur_y += 16;
		}
		unsigned char str[3];
		unsigned char c;
		str[0] = p[i];
		str[1] = p[i + 1];
		str[2] = 0;
		if (str[0] == 0x0d && str[1] == 0x0a)
		{
			by++;
			Raw_y++;
			bx = 0;
			i++;
			cur_y += 8;
			cur_x = 0;
			continue;
		}
		else if (str[0] == '\n')
		{
			if (by >= vinfo->ysize / 16 - 1)
			{
				High_Screen_ne();
				Raw_y++;
				continue;
			}
			by++;
			Raw_y++;
			bx = 0;
			cur_x = 0;
			cur_y += 16;
			continue;
		}
		else if (str[0] == '\b')
		{
			if (bx > 0)
			{
				bx--;
				cur_x -= 8;
				int bmx = bx;
				int bmy = by;
				SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, 0x0, vinfo->xsize);
				bx++;
				cur_x += 8;
				put_sht(sht_scr, bmx * 8, bmy * 16, 1);
				bmx = bx;
				bmy = by;
				SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, 0x0, vinfo->xsize);
				bx++;
				cur_x += 8;
				put_sht(sht_scr, bmx * 8, bmy * 16, 1);
				bx -= 2;
				cur_x -= 16;
			}
			continue;
		}
		if (GetCHorEN(str) == 1)
		{
			int bmx = bx;
			int bmy = by;
			if (bx == vinfo->xsize / 8 - 1)
			{
				by++;
				bx = 0;
				Raw_x = 0;
				Raw_y++;
				cur_x = 0;
				cur_y += 16;
			}

			// PutChineseChar0(scr_buf, vinfo->xsize, bx / 2 + 1, by, (unsigned char)cons_color & 0x0f, *(short *)str);
			PUTCHINESE0(scr_buf, bx * 8, by * 16, (unsigned char)cons_color & 0x0f, *(short *)str, vinfo->xsize);

			i++;
			bx += 2;
			Raw_x += 2;
			put_sht(sht_scr, bmx * 8, bmy * 16, 2);
			cur_x += 16;
			continue;
		}
		c = p[i];
		int bmx = bx;
		int bmy = by;
		SDraw_CHAR(scr_buf, bx, by, c, 0x0f, vinfo->xsize);
		bx++;
		Raw_x++;
		put_sht(sht_scr, bmx * 8, bmy * 16, 1);
		cur_x += 8;
	}
}
void HighPutStr(char *string, int length)
{
	char *p = string;
	int i;
	unsigned char cons_color = 0x0f;
	struct VBEINFO *vinfo = VBEINFO_ADDRESS;
	if (vinfo->vram == 0)
	{
		return;
	}
	for (i = 0; i < length; i++)
	{
		// put_box(sht_scr,bx,by,1,0);
		if (p[i] == '\r')
		{
			continue;
		}
		if (by == vinfo->ysize / 16 - 1 && bx == vinfo->xsize / 8)
		{
			High_Screen_ne();
		}
		if (bx == vinfo->xsize / 8)
		{
			by++;
			Raw_y++;
			bx = 0;
			cur_x = 0;
			cur_y += 16;
		}
		unsigned char str[3];
		unsigned char c;
		str[0] = p[i];
		str[1] = p[i + 1];
		str[2] = 0;
		if (str[0] == 0x0d && str[1] == 0x0a)
		{
			by++;
			Raw_y++;
			bx = 0;
			i++;
			cur_y += 8;
			cur_x = 0;
			continue;
		}
		else if (str[0] == '\n')
		{
			if (by >= vinfo->ysize / 16 - 1)
			{
				High_Screen_ne();
				Raw_y++;
				continue;
			}
			by++;
			Raw_y++;
			bx = 0;
			cur_x = 0;
			cur_y += 16;
			continue;
		}
		else if (str[0] == '\b')
		{
			if (bx > 0)
			{
				bx--;
				cur_x -= 8;
				int bmx = bx;
				int bmy = by;
				SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, 0x0, vinfo->xsize);
				bx++;
				cur_x += 8;
				put_sht(sht_scr, bmx * 8, bmy * 16, 1);
				bmx = bx;
				bmy = by;
				SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, 0x0, vinfo->xsize);
				bx++;
				cur_x += 8;
				put_sht(sht_scr, bmx * 8, bmy * 16, 1);
				bx -= 2;
				cur_x -= 16;
			}
			continue;
		}
		if (GetCHorEN(str) == 1)
		{
			int bmx = bx;
			int bmy = by;
			if (bx == vinfo->xsize / 8 - 1)
			{
				by++;
				bx = 0;
				Raw_x = 0;
				Raw_y++;
				cur_x = 0;
				cur_y += 16;
			}

			// PutChineseChar0(scr_buf, vinfo->xsize, bx / 2 + 1, by, (unsigned char)cons_color & 0x0f, *(short *)str);
			PUTCHINESE0(scr_buf, bx * 8, by * 16, (unsigned char)cons_color & 0x0f, *(short *)str, vinfo->xsize);

			i++;
			bx += 2;
			Raw_x += 2;
			put_sht(sht_scr, bmx * 8, bmy * 16, 2);
			cur_x += 16;
			continue;
		}
		c = p[i];
		int bmx = bx;
		int bmy = by;
		SDraw_CHAR(scr_buf, bx, by, c, 0x0f, vinfo->xsize);
		bx++;
		Raw_x++;
		put_sht(sht_scr, bmx * 8, bmy * 16, 1);
		cur_x += 8;
	}
}
void HighGotoXY(int x, int y)
{
	bx = x;
	by = y;
	cur_x = x * 8;
	cur_y = y * 16;
}
void Gar_Test_Task()
{
	int fifo_buf[128];
	struct FIFO32 fifo;
	fifo32_init(&fifo, 128, fifo_buf);
	struct TIMER *timer;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 25);
	int i = 0;
	while (1)
	{
		sheet_slide(sht_cur, cur_x, cur_y);
		if (fifo32_status(&fifo) != 0)
		{
			i = fifo32_get(&fifo);
			if (i == 1)
			{
				sheet_updown(sht_cur, -1);
				timer_init(timer, &fifo, 2);
				timer_settime(timer, 25);
			}
			else if (i == 2)
			{
				sheet_updown(sht_cur, 1);
				timer_init(timer, &fifo, 1);
				timer_settime(timer, 25);
			}
		}
	}
}
unsigned int stack;
void SwitchToHighTextMode()
{
	SwitchVBEMode(MODE_1024X768X256);
	bx = 0;
	by = 0;
	cur_x = 0;
	cur_y = 0;
	int i, fifobuf[128];
	shtctl0 = shtctl_init(vinfo->vram, vinfo->xsize, vinfo->ysize);
	scr_buf = page_kmalloc(vinfo->xsize * vinfo->ysize);
	cur_buf = page_kmalloc(135 * 75);
	sht_scr = sheet_alloc(shtctl0);
	sht_cur = sheet_alloc(shtctl0);
	sheet_setbuf(sht_scr, scr_buf, vinfo->xsize, vinfo->ysize, -1);
	sheet_setbuf(sht_cur, cur_buf, 8, 16, 99);
	memset(scr_buf, 0, vinfo->xsize * vinfo->ysize);
	Draw_Cur(cur_buf, 0, 0, 8);
	sheet_slide(sht_scr, 0, 0);
	sheet_slide(sht_cur, cur_x, cur_y);
	sheet_updown(sht_scr, 0);
	sheet_updown(sht_cur, 1);
	stack = (unsigned int)page_kmalloc(64 * 1024);
	AddTask("t1", 1, 2 * 8, (int)Gar_Test_Task, 1 * 8, 1 * 8, stack + 64 * 1024);
	Raw_x = 0;
	Raw_y = 0;
}
void Close_High_Text_Mode(void)
{
	SubTask(GetTaskForName("t1"));
	sheet_free(sht_scr);
	sheet_free(sht_cur);
	ctl_free(shtctl0);
	page_kfree(stack,64*1024);
	page_kfree((int)scr_buf, vinfo->xsize * vinfo->ysize);
	page_kfree((int)cur_buf, 135 * 75);
}
void GarTest()
{
	SwitchToHighTextMode();
	// fifo32_init(&fifo, 128, fifobuf, 0);
	// HighPrint("TEST，测试打印文字\n换行测试。混合中英文测试English Chinese（英文 中文）");
	// SDraw_CHAR(scr_buf,0,0,'A',0x0f,vinfo->xsize);

	HighPrint("高分辨率模式下的测试命令已经启动。\n如果你看到这个界面，说明VBE功能一切正常。\nPowerint DOS 386 0.5c\n您可以随便输入一些文字。\n");
	HighPrint("注意：在此模式下，您不能随心随意的返回DOS Mode 您只能通过按下主机上的重启按钮，重新引导操作系统，即可回到DOS Mode。\n");
	HighPrint("我们将要开始循环输出“阿这”，如果您输入y，就代表同意我们循环输出阿这，否则我们将退出Gartest命令。^-^\n");
	char ch;
inp:
	ch = getch();
	if (ch == 'n')
	{
		HighPrint("我们将退出Gartest命令。\n");
		HighPrint("我们将会关闭一个为处理光标的任务，然后释放在此模式下所有的图层，并且返回到DOS Mode。这些操作将很快完成\n");
		HighPrint("谢谢您对我们长久以往的支持，再见。\n");
		HighPrint("请按任意键继续...");
		getch();
		HighPrint("\n");
		Close_High_Text_Mode();
		SwitchToText8025_BIOS();
		printf("Gartest done.\nPress Any Key to continue...\n");
		return;
	}
	else if (ch != 'y')
	{
		HighPrint("我们无法理解您的输入，请再输入一次。\n");
		goto inp;
	}
	HighPrint("警告：这将不会留给你退出的机会！\n");
	HighPrint("请按任意键继续...");
	getch();
	for (;;)
	{
		HighPrint("阿这");
	}
}
void PrintChineseChar(int x, int y, char color, unsigned short Cchar)
{
	//提供PUTCHINESE的更深一层调用（XY）
	PUTCHINESE(x * 16, y * 16, color, Cchar);
}
void PrintChineseStr(int x, int y, char color, unsigned char *str)
{
	int i;
	for (i = 0; i < strlen(str); i += 2)
	{
		unsigned char cstr[3] = {str[i], str[i + 1]};
		PrintChineseChar(x + i / 2, y, color, *(short *)(cstr));
	}
}
void Draw_Px(int x, int y, int color)
{

	unsigned char *vram = (unsigned char *)0xA0000;
	unsigned char *p;
	p = vram + (y * 320 + x);
	*p = color;
	return;
}
void Draw_Box(int x, int y, int w, int h, int color)
{

	int i, j;
	for (i = y; i <= h; i++)
	{
		for (j = x; j <= w; j++)
		{
			Draw_Px(j, i, color);
		}
	}
	return;
}
void Draw_Box32(unsigned char *vram, int xsize, int x0, int y0, int x1, int y1, int Cr, int Cg, int Cb)
{
	int i, j;
	for (i = x0; i <= x1; i++)
	{
		for (j = y0; j <= y1; j++)
		{
			Draw_Px_32(vram, i, j, Cr, Cg, Cb, xsize);
		}
	}
}
//绘制字�??
void Draw_Char(int x, int y, char c, int color)
{
	x *= 8;
	y *= 16;
	unsigned char *vram = (unsigned char *)0xA0000;
	unsigned char *p;
	unsigned char *font;
	int i, j;
	font = fopen("A:\\other\\font1.bin");
	//计算字�?�在字库�?的位�?
	font += c * 16;
	// 0x0f f���ַ�����ɫ
	// 0xf0 f�Ǳ�������ɫ
	int char_color = color & 0x0f;
	int bg_color = (color & 0xf0) >> 4;
	if (c == ' ')
	{
		//Ϳ��
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 8; j++)
			{
				// Draw_Px(x + j, y + i, color);
				Draw_Px(x + j, y + i, bg_color);
			}
		}
	}
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (font[i] & (0x80 >> j))
			{
				p = vram + (y + i) * 320 + x + j;
				*p = color;
			}
		}
	}
	return;
}
void Draw_Str(int x, int y, char *str, int color)
{
	int i;
	for (i = 0; i < strlen(str); i++)
	{
		Draw_Char(x + i, y, str[i], color);
	}
	return;
}

static void Gar_putstr(char *string, int length)
{
	char *p = string;
	int i;
	unsigned char cons_color = 0x0f;
	for (i = 0; i != length; i++)
	{
		if (y == 11)
		{
			break;
		}
		if (x == 40)
		{
			y++;
			x = 0;
		}
		unsigned char str[3];
		unsigned char c;
		str[0] = p[i];
		str[1] = p[i + 1];
		str[2] = 0;
		if (str[0] == 0x0d && str[1] == 0x0a)
		{
			y++;
			x = 0;
			i++;
			continue;
		}
		else if (str[0] == '\n')
		{
			y++;
			x = 0;
			continue;
		}
		else if (str[0] == '\b')
		{
			if (x > 0)
			{
				x--;
				Gprint("  ");
				x -= 2;
			}
			continue;
		}
		if (GetCHorEN(str) == 1)
		{
			if (x == 39)
			{
				y++;
				x = 0;
			}
			if (x % 2 != 0)
			{
				PrintChineseChar(x / 2 + 1, y, (unsigned char)cons_color & 0x0f, *(short *)str);
			}
			else
			{
				PrintChineseChar(x / 2, y, (unsigned char)cons_color & 0x0f, *(short *)str);
			}
			i++;
			x += 2;
			continue;
		}
		c = p[i];
		Draw_Char(x, y, c, (unsigned char)cons_color & 0x0f);
		x++;
	}
	if (y < 11)
	{
		goto nomore;
	}
	goto start2;
again2:
	// getch();
	memcpy(0xa0000, 0xa1400, 0xe600);
	int j;
	for (j = 0; j != 0x1400; j++)
	{
		*(char *)(0xae600 + j) = (unsigned char)cons_color >> 4;
	}
	x = 0;
	y = 11;
start2:
	for (; i != length; i++)
	{
		unsigned char str[3];
		unsigned char c;
		str[0] = p[i];
		str[1] = p[i + 1];
		str[2] = 0;
		if (x == 40)
		{
			goto again2;
		}
		if (str[0] == 0x0d && str[1] == 0x0a)
		{
			i += 2;
			goto again2;
		}
		else if (str[0] == '\n')
		{
			i++;
			goto again2;
		}
		else if (str[0] == '\b')
		{
			if (x > 0)
			{
				x--;
				Gprint("  ");
				x -= 2;
			}
			continue;
		}
		if (GetCHorEN(str) == 1)
		{
			if (x > 39)
			{
				goto again2;
			}
			if (x % 2 != 0)
			{
				PrintChineseChar(x / 2 + 1, y, (unsigned char)cons_color & 0x0f, *(short *)str);
			}
			else
			{
				PrintChineseChar(x / 2, y, (unsigned char)cons_color & 0x0f, *(short *)str);
			}
			i++;
			x += 2;
			continue;
		}
		c = p[i];
		Draw_Char(x, y, c, (unsigned char)cons_color & 0x0f);
		x++;
	}
nomore:
	return;
}
void Gprint(char *string)
{
	Gar_putstr(string, strlen(string));
}
void Gprintf(char *string, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, string);
	vsprintf(buf, string, ap);
	va_end(ap);
	Gar_putstr(buf, strlen(buf));
}
void PUTCHINESE(int x, int y, char color, unsigned short CH)
{
	int i, j, k, offset;
	int flag;
	unsigned char buffer[32];
	unsigned char word[2] = {CH & 0xff, (CH & 0xff00) >> 8}; // 鏀规垚浣犵殑杞?鐮佸悗鐨勬眽瀛楃紪缈???
	unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	unsigned char *p = fopen("A:\\other\\HZK16");
	offset = (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
	p = p + offset;
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
					Draw_Px(x + i + j * 8, y + k, color);
				}
			}
		}
	}
}
void Draw_Px_32(unsigned char *buf, int x, int y, char r, char g, char b, int xsize)
{
	// 绘制一个像素点
	int offset = (x + y * xsize) * 3;
	buf[offset + 0] = b;
	buf[offset + 1] = g;
	buf[offset + 2] = r;
}
//画圆
void Draw_Circle_32(int x, int y, int r, char Cr, char Cg, char Cb, int xsize, unsigned char *vram)
{
	// 画一个圆
	int i, j;
	int flag;
	//右下角
	for (i = 0; i <= r; i++)
	{
		for (j = 0; j <= r; j++)
		{
			flag = (i * i + j * j) <= (r * r);
			if (flag)
			{
				Draw_Px_32(vram, x + i, y + j, Cr, Cg, Cb, xsize);
			}
		}
	}
	//左下角
	for (i = 0; i <= r; i++)
	{
		for (j = 0; j <= r; j++)
		{
			flag = (i * i + j * j) <= (r * r);
			if (flag)
			{
				Draw_Px_32(vram, x - i, y + j, Cr, Cg, Cb, xsize);
			}
		}
	}
	//左上角
	for (i = 0; i <= r; i++)
	{
		for (j = 0; j <= r; j++)
		{
			flag = (i * i + j * j) <= (r * r);
			if (flag)
			{
				Draw_Px_32(vram, x - i, y - j, Cr, Cg, Cb, xsize);
			}
		}
	}
	//右上角
	for (i = 0; i <= r; i++)
	{
		for (j = 0; j <= r; j++)
		{
			flag = (i * i + j * j) <= (r * r);
			if (flag)
			{
				Draw_Px_32(vram, x + i, y - j, Cr, Cg, Cb, xsize);
			}
		}
	}
}
struct _RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
//倒转数据
void Reverse_Data(struct _RGB *buf, int len)
{
	int i, j;
	struct _RGB temp;
	for (i = 0; i < len / 2; i++)
	{
		temp = buf[i];
		buf[i] = buf[len - 1 - i];
		buf[len - 1 - i] = temp;
	}
}
//倒转数据，左右对调
void Reverse_Data_LR(struct _RGB *buf, int len)
{
	int i, j;
	struct _RGB temp;
	for (i = 0; i < len / 2; i++)
	{
		temp = buf[i];
		buf[i] = buf[len - 1 - i];
		buf[len - 1 - i] = temp;
	}
	for (i = 0; i < len; i++)
	{
		temp = buf[i];
		buf[i] = buf[len - 1 - i];
		buf[len - 1 - i] = temp;
	}
}
void BMPVIEW32(char *path, unsigned char *vram, int xsize)
{
	// 显示一个BMP图片
	int i, j, k;
	int offset;
	int x, y;
	int width, height;
	int r, g, b;
	unsigned char buffer[54];
	unsigned char *p = fopen(path);
	for (i = 0; i < 54; i++)
	{
		buffer[i] = p[i];
	}
	width = *(int *)&buffer[18];
	height = *(int *)&buffer[22];
	offset = *(int *)&buffer[10];
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			x = j;
			y = height - i - 1;
			k = offset + (i * width + j) * 3;
			b = p[k + 0];
			g = p[k + 1];
			r = p[k + 2];
			Draw_Px_32(vram, x, y, r, g, b, xsize);
		}
	}
}
void jpgview32(char *path, unsigned char *vram, int xsize)
{
	struct FILEINFO *fileinfo = Get_File_Address(path);
	char *buf = fopen(path);
	int info[8];
	struct DLL_STRPICENV env;
	struct RGB *picbuf,*q;
	picbuf = page_vmalloc(sizeof(struct RGB)*1024*768);
	if (info_JPEG(&env, info, fileinfo->size, buf) == 0)
	{
		/*也不是JPEG */
		SwitchToText8025_BIOS();
		clear();
		printf("format error\n");
		return;
	}
	if (info[2] > 1024 || info[3] > 768) {
		/*图片太大*/
		SwitchToText8025_BIOS();
		clear();
		printf("picture too big,is %dx%d\n", info[2], info[3]);
		return;
	}
	int i =  decode0_JPEG(&env, fileinfo->size, buf, 4, (char *) picbuf, 0);
	if(i!=0)
	{
		printf("decode error \n");
		return;
	}
	for (i = 0; i < info[3]; i++) {
		q = picbuf + i * info[2];
		for (int j = 0; j < info[2]; j++) {
			Draw_Px_32(vram, j, i, q[j].r, q[j].g, q[j].b, xsize);
		}
	}
	page_vfree((int)picbuf, sizeof(struct RGB)*1024*768);
}
// 转换灰白色
void black(unsigned char *vram, int xsize, int ysize)
{
	int i, j;
	int x, y;
	int r, g, b;
	for (i = 0; i < ysize; i++)
	{
		for (j = 0; j < xsize; j++)
		{
			x = j;
			y = ysize - i - 1;
			r = vram[(xsize * y + x) * 3 + 0];
			g = vram[(xsize * y + x) * 3 + 1];
			b = vram[(xsize * y + x) * 3 + 2];
			r = (r + g + b) / 3;
			g = r;
			b = r;
			Draw_Px_32(vram, x, y, r, g, b, xsize);
		}
	}
}
