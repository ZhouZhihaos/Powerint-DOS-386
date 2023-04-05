#include <dos.h>
#include <drivers.h>
static void set_plane(unsigned p);
unsigned char g_320x200x256[] = {
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00};
unsigned char table_rgb[16 * 3] = {0x00, 0x00, 0x00,  // 0
                                   0x00, 0x00, 167,   // 1
                                   0x00, 0xff, 0x00,  // 2
                                   39,   39,   39,    // 3
                                   167,  0x00, 0x00,  // 4
                                   253,  253,  85,    // 5
                                   193,  153,  0,     // 6
                                   0xc6, 0xc6, 0xc6,  // 7
                                   0x84, 0x84, 0x84,  // 8
                                   0x84, 0x00, 0x00,  // 9
                                   22,   198,  12,    // 10 a
                                   0x84, 0x84, 0x00,  // 11 b
                                   0x00, 0x00, 0x84,  // 12 c
                                   0x84, 0x00, 0x84,  // 13 d
                                   253,  253,  85,    // 14 e
                                   0xFF, 0xFF, 0xFF}; // 15 f
unsigned char g_80x25_text[] = {
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 0x00, 0x4F, 0x0D, 0x0E,
    0x00, 0x00, 0x00, 0x50, 0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x0C, 0x00, 0x0F, 0x08, 0x00};
char *text_mode_data;
void write_regs(unsigned char *regs) {
  unsigned int i;

  /* 写MISCELLANEOUS寄存器 */
  io_out8(VGA_MISC_WRITE, *regs);
  regs++;
  /* 写SEQUENCER寄存器 */
  for (i = 0; i < VGA_NUM_SEQ_REGS; i++) {
    io_out8(VGA_SEQ_INDEX, i);
    io_out8(VGA_SEQ_DATA, *regs);
    regs++;
  }
  /* 解锁 CRTC 寄存器 */
  io_out8(VGA_CRTC_INDEX, 0x03);
  io_out8(VGA_CRTC_DATA, io_in8(VGA_CRTC_DATA) | 0x80);
  io_out8(VGA_CRTC_INDEX, 0x11);
  io_out8(VGA_CRTC_DATA, io_in8(VGA_CRTC_DATA) & ~0x80);
  /* 确保它们保持解锁状态 */
  regs[0x03] |= 0x80;
  regs[0x11] &= ~0x80;
  /* 写CRTC寄存器 */
  for (i = 0; i < VGA_NUM_CRTC_REGS; i++) {
    io_out8(VGA_CRTC_INDEX, i);
    io_out8(VGA_CRTC_DATA, *regs);
    regs++;
  }
  /* 写GRAPHICS CONTROLLER寄存器 */
  for (i = 0; i < VGA_NUM_GC_REGS; i++) {
    io_out8(VGA_GC_INDEX, i);
    io_out8(VGA_GC_DATA, *regs);
    regs++;
  }
  /* 写ATTRIBUTE CONTROLLER寄存器 */
  for (i = 0; i < VGA_NUM_AC_REGS; i++) {
    (void)io_in8(VGA_INSTAT_READ);
    io_out8(VGA_AC_INDEX, i);
    io_out8(VGA_AC_WRITE, *regs);
    regs++;
  }
  /* 锁定 16 色调色板和空白显示 */
  (void)io_in8(VGA_INSTAT_READ);
  io_out8(VGA_AC_INDEX, 0x20);
}
void init_palette(void) {
  set_palette(0, 15, table_rgb);
  return;
}
unsigned get_fb_seg(void) {
  unsigned seg;

  io_out8(VGA_GC_INDEX, 6);
  seg = io_in8(VGA_GC_DATA);
  seg >>= 2;
  seg &= 3;
  switch (seg) {
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
void vmemwr(unsigned dst_off, unsigned char *src, unsigned count) {
  _vmemwr(get_fb_seg(), dst_off, src, count);
}
void write_font(unsigned char *buf, unsigned font_height) {
  unsigned char seq2, seq4, gc4, gc5, gc6;
  unsigned i;

  /* 保存寄存器
set_plane() 修改了 GC4 和 SEQ2，因此也保存它们 */
  io_out8(VGA_SEQ_INDEX, 2);
  seq2 = io_in8(VGA_SEQ_DATA);

  io_out8(VGA_SEQ_INDEX, 4);
  seq4 = io_in8(VGA_SEQ_DATA);
  /* 关闭奇数寻址（设置平面寻址）
假设：chain-4 寻址已关闭 */
  io_out8(VGA_SEQ_DATA, seq4 | 0x04);

  io_out8(VGA_GC_INDEX, 4);
  gc4 = io_in8(VGA_GC_DATA);

  io_out8(VGA_GC_INDEX, 5);
  gc5 = io_in8(VGA_GC_DATA);
  /* 关闭偶数寻址 */
  io_out8(VGA_GC_DATA, gc5 & ~0x10);

  io_out8(VGA_GC_INDEX, 6);
  gc6 = io_in8(VGA_GC_DATA);
  /* 关闭偶数寻址 */
  io_out8(VGA_GC_DATA, gc6 & ~0x02);
  /* 将字体写入平面 P4 */
  set_plane(2);
  /* 写字体 0 */
  for (i = 0; i < 256; i++) {
    vmemwr(16384u * 0 + i * 32, buf, font_height);
    buf += font_height;
  }
#if 0
/* 写字体 1 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
  /* 恢复寄存器 */
  io_out8(VGA_SEQ_INDEX, 2);
  io_out8(VGA_SEQ_DATA, seq2);
  io_out8(VGA_SEQ_INDEX, 4);
  io_out8(VGA_SEQ_DATA, seq4);
  io_out8(VGA_GC_INDEX, 4);
  io_out8(VGA_GC_DATA, gc4);
  io_out8(VGA_GC_INDEX, 5);
  io_out8(VGA_GC_DATA, gc5);
  io_out8(VGA_GC_INDEX, 6);
  io_out8(VGA_GC_DATA, gc6);
}
void set_palette(int start, int end, unsigned char *rgb) {
  int i, eflags;
  eflags = io_load_eflags();
  io_cli();
  io_out8(0x03c8, start);
  for (i = start; i <= end; i++) {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);
  return;
}
void Set_Font(char *file) {
  unsigned rows, cols, ht, i;
  FILE *fp = fopen(file, "r");
  char *g_8x16_font = fp->buffer;
  cols = 80;
  rows = 25;
  ht = 16;
  //设置字库
  write_font(g_8x16_font, 16);
  pokew(0x40, 0x4A, cols);            /* 屏幕上的列 */
  pokew(0x40, 0x4C, cols * rows * 2); /* 帧缓冲器大小 */
  pokew(0x40, 0x50, 0);               /* 光标位置 */
  pokeb(0x40, 0x60, ht - 1);          /* 光标形状 */
  pokeb(0x40, 0x61, ht - 2);
  pokeb(0x40, 0x84, rows - 1); /* 屏幕上的行数 - 1 */
  pokeb(0x40, 0x85, ht);       /* 字符高度 */
                               /* 为所有文本设置黑底白字属性 */
}
void SwitchTo320X200X256() {
  write_regs(g_320x200x256);
  init_palette();
  char *p = 0xa0000;
  int i, j;
  //写入新数据
  for (i = 0; i < 200; i++) {
    for (j = 0; j < 320; j++) {
      p[i * 320 + j] = 0x00;
    }
  }
}
void pokeb(int setmentaddr, int offset, char value) {
  *(char *)(setmentaddr * 0x10 + offset) = value;
}
void pokew(int setmentaddr, int offset, short value) {
  *(short *)(setmentaddr * 0x10 + offset) = value;
}
static void set_plane(unsigned p) {
  unsigned char pmask;

  p &= 3;
  pmask = 1 << p;
  /* 设置读取平面 */
  io_out8(VGA_GC_INDEX, 4);
  io_out8(VGA_GC_DATA, p);
  /* 设置写入平面 */
  io_out8(VGA_SEQ_INDEX, 2);
  io_out8(VGA_SEQ_DATA, pmask);
}
void SwitchToText8025() {
  unsigned rows, cols, ht, i;
  char *g_8x16_font = ascfont;
  write_regs(g_80x25_text);
  // 重新初始化调色板
  init_palette();
  cols = 80;
  rows = 25;
  ht = 16;
  //设置字库
  write_font(g_8x16_font, 16);
  pokew(0x40, 0x4A, cols);            /* 屏幕上的列 */
  pokew(0x40, 0x4C, cols * rows * 2); /* 帧缓冲器大小 */
  pokew(0x40, 0x50, 0);               /* 光标位置 */
  pokeb(0x40, 0x60, ht - 1);          /* 光标形状 */
  pokeb(0x40, 0x61, ht - 2);
  pokeb(0x40, 0x84, rows - 1); /* 屏幕上的行数 - 1 */
  pokeb(0x40, 0x85, ht);       /* 字符高度 */
                               /* 为所有文本设置黑底白字属性 */
  for (i = 0; i < cols * rows; i++)
    pokeb(0xB800, i * 2 + 1, 7);
  clear(); // 清屏
}