#include "system.h"
unsigned char g_320x200x256[] =
    {
        /* MISC */
        0x63,
        /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00};
unsigned char g_80x25_text[] =
    {
        /* MISC */
        0x67,
        /* SEQ */
        0x03, 0x00, 0x03, 0x00, 0x02,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
        0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
        0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
        0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00};
char *text_mode_data;
void write_regs(unsigned char *regs)
{
    unsigned int i;

    /* write MISCELLANEOUS reg */
    outportb(VGA_MISC_WRITE, *regs);
    regs++;
    /* write SEQUENCER regs */
    for (i = 0; i < VGA_NUM_SEQ_REGS; i++)
    {
        outportb(VGA_SEQ_INDEX, i);
        outportb(VGA_SEQ_DATA, *regs);
        regs++;
    }
    /* unlock CRTC registers */
    outportb(VGA_CRTC_INDEX, 0x03);
    outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
    outportb(VGA_CRTC_INDEX, 0x11);
    outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);
    /* make sure they remain unlocked */
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;
    /* write CRTC regs */
    for (i = 0; i < VGA_NUM_CRTC_REGS; i++)
    {
        outportb(VGA_CRTC_INDEX, i);
        outportb(VGA_CRTC_DATA, *regs);
        regs++;
    }
    /* write GRAPHICS CONTROLLER regs */
    for (i = 0; i < VGA_NUM_GC_REGS; i++)
    {
        outportb(VGA_GC_INDEX, i);
        outportb(VGA_GC_DATA, *regs);
        regs++;
    }
    /* write ATTRIBUTE CONTROLLER regs */
    for (i = 0; i < VGA_NUM_AC_REGS; i++)
    {
        (void)inportb(VGA_INSTAT_READ);
        outportb(VGA_AC_INDEX, i);
        outportb(VGA_AC_WRITE, *regs);
        regs++;
    }
    /* lock 16-color palette and unblank display */
    (void)inportb(VGA_INSTAT_READ);
    outportb(VGA_AC_INDEX, 0x20);
}
//现在，让我们进入图形模式！
void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, /*  0:黑 */
        0xff, 0x00, 0x00, /*  1:梁红 */
        0x00, 0xff, 0x00, /*  2:亮绿 */
        0xff, 0xff, 0x00, /*  3:亮黄 */
        0x00, 0x00, 0xff, /*  4:亮蓝 */
        0xff, 0x00, 0xff, /*  5:亮紫 */
        0x00, 0xff, 0xff, /*  6:浅亮蓝 */
        0xff, 0xff, 0xff, /*  7:白 */
        0xc6, 0xc6, 0xc6, /*  8:亮灰 */
        0x84, 0x00, 0x00, /*  9:暗红 */
        0x00, 0x84, 0x00, /* 10:暗绿 */
        0x84, 0x84, 0x00, /* 11:暗黄 */
        0x00, 0x00, 0x84, /* 12:暗青 */
        0x84, 0x00, 0x84, /* 13:暗紫 */
        0x00, 0x84, 0x84, /* 14:浅暗蓝 */
        0x84, 0x84, 0x84  /* 15:暗灰 */
    };
    set_palette(0, 15, table_rgb);
    return;

    /* C语言中的static char语句只能用于数据，相当于汇编中的DB指令 */
}
static unsigned get_fb_seg(void)
{
    unsigned seg;

    outportb(VGA_GC_INDEX, 6);
    seg = inportb(VGA_GC_DATA);
    seg >>= 2;
    seg &= 3;
    switch (seg)
    {
    case 0:
    case 1:
        seg = 0xA000;
        break;
    case 2:
        seg = 0xB000;
        break;
    case 3:
        seg = 0xB800;
        break;
    }
    return seg;
}
static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
    _vmemwr(get_fb_seg(), dst_off, src, count);
}
static void write_font(unsigned char *buf, unsigned font_height)
{
    unsigned char seq2, seq4, gc4, gc5, gc6;
    unsigned i;

    /* save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
    outportb(VGA_SEQ_INDEX, 2);
    seq2 = inportb(VGA_SEQ_DATA);

    outportb(VGA_SEQ_INDEX, 4);
    seq4 = inportb(VGA_SEQ_DATA);
    /* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
    outportb(VGA_SEQ_DATA, seq4 | 0x04);

    outportb(VGA_GC_INDEX, 4);
    gc4 = inportb(VGA_GC_DATA);

    outportb(VGA_GC_INDEX, 5);
    gc5 = inportb(VGA_GC_DATA);
    /* turn off even-odd addressing */
    outportb(VGA_GC_DATA, gc5 & ~0x10);

    outportb(VGA_GC_INDEX, 6);
    gc6 = inportb(VGA_GC_DATA);
    /* turn off even-odd addressing */
    outportb(VGA_GC_DATA, gc6 & ~0x02);
    /* write font to plane P4 */
    set_plane(2);
    /* write font 0 */
    for (i = 0; i < 256; i++)
    {
        vmemwr(16384u * 0 + i * 32, buf, font_height);
        buf += font_height;
    }
#if 0
/* write font 1 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
    /* restore registers */
    outportb(VGA_SEQ_INDEX, 2);
    outportb(VGA_SEQ_DATA, seq2);
    outportb(VGA_SEQ_INDEX, 4);
    outportb(VGA_SEQ_DATA, seq4);
    outportb(VGA_GC_INDEX, 4);
    outportb(VGA_GC_DATA, gc4);
    outportb(VGA_GC_INDEX, 5);
    outportb(VGA_GC_DATA, gc5);
    outportb(VGA_GC_INDEX, 6);
    outportb(VGA_GC_DATA, gc6);
}
void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags(); /* 记录中断许可标志的值 */
    io_cli();                  /* 将中断许可标志置为0,禁止中断 */
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /* 复原中断许可标志 */
    return;
}
void SwitchTo320X200X256()
{
    write_regs(g_320x200x256);
    init_palette();
    char *p = 0xa0000;
    int i, j;
    //写入新数据
    for (i = 0; i < 200; i++)
    {
        for (j = 0; j < 320; j++)
        {
            p[i * 320 + j] = 0x00;
        }
    }
}
void pokeb(int setmentaddr, int offset, char value)
{
    *(char *)(setmentaddr * 0x10 + offset) = value;
}
void pokew(int setmentaddr, int offset, short value)
{
    *(short *)(setmentaddr * 0x10 + offset) = value;
}
static void set_plane(unsigned p)
{
    unsigned char pmask;

    p &= 3;
    pmask = 1 << p;
    /* set read plane */
    outportb(VGA_GC_INDEX, 4);
    outportb(VGA_GC_DATA, p);
    /* set write plane */
    outportb(VGA_SEQ_INDEX, 2);
    outportb(VGA_SEQ_DATA, pmask);
}

void SwitchToText8025()
{
    unsigned rows, cols, ht, i;
	char *g_8x16_font=fopen("font1.bin");
    write_regs(g_80x25_text);
    init_palette();
    cols = 80;
    rows = 25;
    ht = 16;
    //设置字库
    write_font(g_8x16_font, 16);
    pokew(0x40, 0x4A, cols);            /* columns on screen */
    pokew(0x40, 0x4C, cols * rows * 2); /* framebuffer size */
    pokew(0x40, 0x50, 0);               /* cursor pos'n */
    pokeb(0x40, 0x60, ht - 1);          /* cursor shape */
    pokeb(0x40, 0x61, ht - 2);
    pokeb(0x40, 0x84, rows - 1); /* rows on screen - 1 */
    pokeb(0x40, 0x85, ht);       /* char height */
                                 /* set white-on-black attributes for all text */
    for (i = 0; i < cols * rows; i++)
        pokeb(0xB800, i * 2 + 1, 7);
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
	for(i = y; i <= h; i++)
	{
		for(j = x; j <= w; j++)
		{
			Draw_Px(j, i, color);
		}
	}
	return;
}
//绘制字符
void Draw_Char(int x, int y, char c, int color)
{
	x *=8;
	y *=16;
	unsigned char *vram = (unsigned char *)0xA0000;
	unsigned char *p;
	unsigned char *font;
	int i, j;
	font =fopen("font1.bin");
	//计算字符在字库中的位置
	font +=c*16;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 8; j++)
		{
			if(font[i] & (0x80 >> j))
			{
				p = vram + (y + i) * 320 + x + j;
				*p = color;
			}
		}
	}
	return;
}
void Draw_Str(int x,int y,char *str,int color)
{
	int i;
	for (i = 0; i < strlen(str); i++)
	{
		Draw_Char(x+i,y,str[i],color);
	}
	return;
}