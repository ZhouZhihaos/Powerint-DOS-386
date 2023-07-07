#include <io.h>
// 画空心圆
void Draw_Circle_Hollow_32(unsigned char* vram,
                           int xsize,
                           int x,
                           int y,
                           int r,
                           int Cr,
                           int Cg,
                           int Cb,
                           int br,
                           int bg,
                           int bb) {
  int i, j;
  int flag;
  for (i = 0; i <= r; i++) {
    for (j = 0; j <= r; j++) {
      flag = (i * i + j * j) <= (r * r);
      if (flag) {
        Draw_Px_32(vram, x + i, y + j, Cr, Cg, Cb, xsize);
        Draw_Px_32(vram, x - i, y + j, Cr, Cg, Cb, xsize);
        Draw_Px_32(vram, x + i, y - j, Cr, Cg, Cb, xsize);
        Draw_Px_32(vram, x - i, y - j, Cr, Cg, Cb, xsize);
      }
    }
  }
  //把中间涂黑
  r = r - 5;
  for (i = 0; i <= r; i++) {
    for (j = 0; j <= r; j++) {
      flag = (i * i + j * j) <= (r * r);
      if (flag) {
        Draw_Px_32(vram, x + i, y + j, br, bg, bb, xsize);
        Draw_Px_32(vram, x - i, y + j, br, bg, bb, xsize);
        Draw_Px_32(vram, x + i, y - j, br, bg, bb, xsize);
        Draw_Px_32(vram, x - i, y - j, br, bg, bb, xsize);
      }
    }
  }
}
// 转换灰白色
void black(unsigned char* vram, int xsize, int ysize) {
  int i, j;
  int x, y;
  int r, g, b;
  for (i = 0; i < ysize; i++) {
    for (j = 0; j < xsize; j++) {
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
void PUTCHINESE(int x, int y, char color, unsigned short CH) {
  int i, j, k, offset;
  int flag;
  unsigned char buffer[32];
  unsigned char word[2] = {CH & 0xff, (CH & 0xff00) >> 8};
  unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
  unsigned char* p = hzkfont;
  offset =
      (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
  p = p + offset;
  for (i = 0; i < 32; i++) {
    buffer[i] = p[i];
  }
  for (k = 0; k < 16; k++) {
    for (j = 0; j < 2; j++) {
      for (i = 0; i < 8; i++) {
        flag = buffer[k * 2 + j] & key[i];
        if (flag) {
          Draw_Px(x + i + j * 8, y + k, color);
        }
      }
    }
  }
}
void PUTCHINESE0(vram_t* vram,
                 int x,
                 int y,
                 color_t color,
                 unsigned short CH,
                 int xsize) {
  int i, j, k, offset;
  int flag;
  unsigned char buffer[32];
  unsigned char word[2] = {CH & 0xff,
                           (CH & 0xff00) >> 8};  // 将字符转换为两个字节
  unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
  unsigned char* p = hzkfont;
  offset =
      (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
  p = p + offset;
  //读取，并写入到vram中
  for (i = 0; i < 32; i++) {
    buffer[i] = p[i];
  }
  for (k = 0; k < 16; k++) {
    for (j = 0; j < 2; j++) {
      for (i = 0; i < 8; i++) {
        flag = buffer[k * 2 + j] & key[i];
        if (flag) {
          // Draw_Px(x + i + j * 8, y + k, color);
          vram[(y + k) * xsize + (x + i + j * 8)] = color;
        }
      }
    }
  }
}
void PrintChineseChar(int x, int y, char color, unsigned short Cchar) {
  //提供PUTCHINESE的更深一层调用（XY）
  PUTCHINESE(x * 16, y * 16, color, Cchar);
}
void PutChineseChar0(vram_t* vram,
                     int xsize,
                     int x,
                     int y,
                     color_t color,
                     unsigned short cChar) {
  PUTCHINESE0(vram, x * 16, y * 16, color, cChar, xsize);
}
void PutChineseStr0(vram_t* vram,
                    int xsize,
                    int x,
                    int y,
                    color_t color,
                    unsigned char* str) {
  int i;
  for (i = 0; i < strlen(str); i += 2) {
    unsigned char cstr[3] = {str[i], str[i + 1]};
    PUTCHINESE0(vram, x + i * 8, y, color, *(short*)(cstr), xsize);
  }
}
void PrintChineseStr(int x, int y, char color, unsigned char* str) {
  int i;
  for (i = 0; i < strlen(str); i += 2) {
    unsigned char cstr[3] = {str[i], str[i + 1]};
    PrintChineseChar(x + i / 2, y, color, *(short*)(cstr));
  }
}
void Draw_Px(int x, int y, char color) {
  unsigned char* vram = (vram_t*)0xA0000;
  unsigned char* p;
  p = vram + (y * 320 + x);
  *p = color;
  return;
}
void Draw_Px_32(unsigned char* buf,
                int x,
                int y,
                char r,
                char g,
                char b,
                int xsize) {
  // 绘制一个像素点
  int offset = (x + y * xsize);
  vram_t* vram = (vram_t*)buf;
  color_t color = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
  vram[offset] = color;
}
void Draw_Box(int x, int y, int w, int h, char color) {
  int i, j;
  for (i = y; i <= h; i++) {
    for (j = x; j <= w; j++) {
      Draw_Px(j, i, color);
    }
  }
  return;
}

void Draw_Box32(unsigned char* vram,
                int xsize,
                int x0,
                int y0,
                int x1,
                int y1,
                int Cr,
                int Cg,
                int Cb) {
  int i, j;
  for (i = x0; i <= x1; i++) {
    for (j = y0; j <= y1; j++) {
      Draw_Px_32(vram, i, j, Cr, Cg, Cb, xsize);
    }
  }
}
void Draw_Char(int x, int y, char c, char color) {
  unsigned char* font;
  font = ascfont;
  font += c * 16;
  unsigned char* vram1 = (unsigned char*)(0xA0000);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      if (font[i] & (0x80 >> j)) {
        vram1[(y + i) * 320 + x + j] = color;
      }
    }
  }
  return;
}
void Draw_Char_32(unsigned char* vram1,
                  int xsiz,
                  char c,
                  int x,
                  int y,
                  int r,
                  int g,
                  int b) {
  unsigned char* vram = (vram_t*)vram1;
  unsigned char* p;
  unsigned char* font;
  int i, j;
  font = ascfont;
  font += c * 16;
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 8; j++) {
      if (font[i] & (0x80 >> j)) {
        Draw_Px_32(vram, x + j, y + i, r, g, b, xsiz);
      }
    }
  }
}
void Draw_Str(int x, int y, char* str, char color) {
  for (int i = 0; i != strlen(str); i++) {
    Draw_Char( x + i * 8, y, str[i], color);
  }
  return;
}
void boxfill(vram_t* vram,
             int xsize,
             color_t c,
             int x0,
             int y0,
             int x1,
             int y1) {
  int x, y;
  for (y = y0; y <= y1; y++) {
    for (x = x0; x <= x1; x++)
      vram[y * xsize + x] = c;
  }
  return;
}
void SDraw_Px(vram_t* vram, int x, int y, color_t color, int xsize) {
  vram[y * xsize + x] = color;
  return;
}
void SDraw_Box(vram_t* vram,
               int x,
               int y,
               int x1,
               int y1,
               color_t color,
               int xsize) {
  int i, j;
  for (i = x; i < x1; i++) {
    for (j = y; j < y1; j++) {
      vram[j*xsize+i] = color;
    }
  }
  return;
}
void SDraw_Char(vram_t* vram1, int x, int y, char c, color_t color, int xsize) {
  // x *= 8;
  // y *= 16;
  unsigned char* font;
  font = ascfont;
  font += c * 16;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 8; j++) {
      if (font[i] & (0x80 >> j)) {
        vram1[(y + i) * xsize + x + j] = color;
      }
    }
  }
  return;
}
void Sputs(vram_t* vram, char* str, int x, int y, color_t col, int xsize) {
  for (int i = 0; i != strlen(str); i++) {
    SDraw_Char(vram, x + i * 8, y, str[i], col, xsize);
  }
}

void putfont(vram_t* vram, int xsize, int x, int y, color_t c, char* font) {
  int i;
  vram_t *p, d /* data */;

  for (i = 0; i < 16; i++) {
    p = vram + (y + i) * xsize + x;
    d = font[i];
    if ((d & 0x80) != 0) {
      p[0] = c;
    }
    if ((d & 0x40) != 0) {
      p[1] = c;
    }
    if ((d & 0x20) != 0) {
      p[2] = c;
    }
    if ((d & 0x10) != 0) {
      p[3] = c;
    }
    if ((d & 0x08) != 0) {
      p[4] = c;
    }
    if ((d & 0x04) != 0) {
      p[5] = c;
    }
    if ((d & 0x02) != 0) {
      p[6] = c;
    }
    if ((d & 0x01) != 0) {
      p[7] = c;
    }
  }
  return;
}

void putfonts_asc(vram_t* vram,
                  int xsize,
                  int x,
                  int y,
                  color_t c,
                  unsigned char* s) {
  int flag = 0;
  char* hankaku = ascfont;
  /* C语言中，字符串都是以0x00结尾 */
  for (; *s != 0x00; s++) {
    if (*s > 0x80 && (*(s + 1) != 0x00 || flag)) {
      if (flag) {
        s--;
        PUTCHINESE0(vram, x, y, c, *(unsigned short*)(s), xsize);
        x += 16;
        flag = 0;
        s++;
      } else {
        flag = 1;
      }
      continue;
    }
    putfont(vram, xsize, x, y, c, hankaku + *s * 16);
    x += 8;
  }
  return;
}
void putfonts_asc_sht(struct SHEET* sht,
                      int x,
                      int y,
                      color_t c,
                      color_t b,
                      char* s,
                      int l) {
  boxfill(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
  putfonts_asc(sht->buf, sht->bxsize, x, y, c, s);
  sheet_refresh(sht, x, y, x + l * 8, y + 16);
  return;
}
void Draw_Line_32(uint8_t* Buffer,
                  int x1,
                  int y1,
                  int x2,
                  int y2,
                  int r,
                  int g,
                  int b,
                  int xsize) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  int x = x1;
  int y = y1;
  int x_inc = 1;
  int y_inc = 1;
  int err = 0;
  int dx2 = dx * 2;
  int dy2 = dy * 2;
  if (dx < 0) {
    x_inc = -1;
    dx = -dx;
    dx2 = -dx2;
  }
  if (dy < 0) {
    y_inc = -1;
    dy = -dy;
    dy2 = -dy2;
  }
  if (dx > dy) {
    err = dx;
    while (x != x2) {
      Draw_Px_32(Buffer, x, y, r, g, b, xsize);
      err -= dy2;
      if (err < 0) {
        y += y_inc;
        err += dx2;
      }
      x += x_inc;
    }
  } else {
    err = dy;
    while (y != y2) {
      Draw_Px_32(Buffer, x, y, r, g, b, xsize);
      err -= dx2;
      if (err < 0) {
        x += x_inc;
        err += dy2;
      }
      y += y_inc;
    }
  }
  Draw_Px_32(Buffer, x, y, r, g, b, xsize);
}
void Draw_Line(vram_t* Buffer,
               int x1,
               int y1,
               int x2,
               int y2,
               color_t c,
               int xsize) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  int x = x1;
  int y = y1;
  int x_inc = 1;
  int y_inc = 1;
  int err = 0;
  int dx2 = dx * 2;
  int dy2 = dy * 2;
  if (dx < 0) {
    x_inc = -1;
    dx = -dx;
    dx2 = -dx2;
  }
  if (dy < 0) {
    y_inc = -1;
    dy = -dy;
    dy2 = -dy2;
  }
  if (dx > dy) {
    err = dx;
    while (x != x2) {
      Buffer[x + y * xsize] = c;
      err -= dy2;
      if (err < 0) {
        y += y_inc;
        err += dx2;
      }
      x += x_inc;
    }
  } else {
    err = dy;
    while (y != y2) {
      Buffer[x + y * xsize] = c;
      err -= dx2;
      if (err < 0) {
        x += x_inc;
        err += dy2;
      }
      y += y_inc;
    }
  }
  Buffer[x + y * xsize] = c;
}
