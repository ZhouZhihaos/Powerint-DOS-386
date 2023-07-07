#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate
                                     // implementation
#include <stdarg.h>
#include <syscall.h>
#include "stb_ttf.h"

char* ttf_buffer;
#define argb(a, r, g, b) ((a) << 24 | (r) << 16 | (g) << 8 | (b))
uint32_t LCD_AlphaBlend(uint32_t foreground_color,
                        uint32_t background_color,
                        uint8_t alpha) {
    uint8_t* fg = (uint8_t*)&foreground_color;
    uint8_t* bg = (uint8_t*)&background_color;

    uint32_t rb = (((uint32_t)(*fg & 0xFF) * alpha) + ((uint32_t)(*bg & 0xFF) * (256 - alpha))) >> 8;
    uint32_t g = (((uint32_t)(*(fg + 1) & 0xFF) * alpha) + ((uint32_t)(*(bg + 1) & 0xFF) * (256 - alpha))) >> 8;
    uint32_t a = (((uint32_t)(*(fg + 2) & 0xFF) * alpha) + ((uint32_t)(*(bg + 2) & 0xFF) * (256 - alpha))) >> 8;

    return (rb & 0xFF) | ((g & 0xFF) << 8) | ((a & 0xFF) << 16);
}
stbtt_fontinfo font;
unsigned char* bitmap;
unsigned int* vram_buffer;
int w, h, i, j, s = (20);
float xpos = 2;
int y_shift = 0;
int height = 0;
int width = 0;
int b;
void TTF_Draw_Char(int x, int y, int ch, unsigned int bc,unsigned fc) {
  int x0, y0, x1, y1;
  stbtt_GetCodepointBitmapBoxSubpixel(
      &font, ch, stbtt_ScaleForPixelHeight(&font, s),
      stbtt_ScaleForPixelHeight(&font, s), 0, 0, &x0, &y0, &x1, &y1);
  // printf("y1=%d y0=%d,x1=%d,x0=%d\n", y1, y0, x1, x0);
  bitmap = stbtt_GetCodepointBitmap(
      &font, 0, stbtt_ScaleForPixelHeight(&font, s), ch, &w, &h, 0, 0);
  unsigned int* buf1 = malloc(w * h * sizeof(unsigned int));
  for (j = 0; j < h; ++j) {
    for (i = 0; i < w; ++i) {
      buf1[j * w + i] = LCD_AlphaBlend(fc, bc, bitmap[j * w + i]);
    }
  }
  VBESetBuffer(((int)x0 + x), (b + y0) + y, w, h, buf1);
  free(buf1);
  free(bitmap);
}
void TTF_Print(char* buf) {
  int start_x = 0;
  for (int l = 0; l < strlen(buf); l++) {
    if (xpos + width > 1024 || buf[l] == '\n') {
      y_shift += height;
      xpos = 2;
      if (buf[l] == '\n') {
        if (y_shift + height > 768) {
          roll(height);  // 滚动屏幕
          xpos = 2;
          y_shift -= height;
        }
        free(bitmap);
        continue;
      }
    }
    if (y_shift + height> 768) {
      roll(height);  // 滚动屏幕
      xpos = 2;
      y_shift -= height;
    }
    if (buf[l] == '\b') {
      if (xpos != 2) {
        xpos -= width;
        VBEDraw_Box(xpos, y_shift, xpos + width + 1, y_shift + height + 1, 0x0);
      }
      free(bitmap);
      continue;
    }
    TTF_Draw_Char(xpos,y_shift,buf[l],0x0,0x00ffffff);
    xpos += width;
    // y_shift += height;
  }
}
int ttf_printf(const char* format, ...) {
  int len;
  va_list ap;
  va_start(ap, format);
  char buf[1024];
  len = vsprintf(buf, format, ap);
  TTF_Print(buf);
  va_end(ap);
  return len;
}
void set_size(int s1) {
  float sc;
  s = s1;
  sc = stbtt_ScaleForPixelHeight(&font, s);
  int advance, lsb;
  stbtt_GetCodepointHMetrics(&font, 'A', &advance, &lsb);
  width = advance * sc + stbtt_GetCodepointKernAdvance(&font, 'A', 'B') * sc;
  int as, lg, des;
  stbtt_GetFontVMetrics(&font, &as, &des, &lg);
  b = (int)((float)as * sc);
  height = (int)((float)(as - des + lg) * sc);
}
int main(int argc, char** argv) {
  ttf_buffer = malloc(filesize("font.ttf"));
  unsigned char buf[100];
  printf("Reading font...");
  api_ReadFile("font.ttf", ttf_buffer);
  printf("Done.\n");
  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));
  system("PAUSE");

  printf("width=%d height=%d\n", width, height);
  system("PAUSE");
  set_mode(1024, 768);  // 设置模式
  set_size(50);
  for (;;) {
    ttf_printf("%c",getch());
  }
  for (;;)
    ;
  return 0;
}
