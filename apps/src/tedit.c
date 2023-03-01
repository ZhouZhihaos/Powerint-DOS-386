// I haven't actually tested this yet, this is just to make sure it compiles

#include <ctype.h>  // isspace
#include <gui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // memmove
#include <syscall.h>

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_STRING text_control
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate
                                     // implementation
#include "stb_ttf.h"
stbtt_fontinfo font;
unsigned char* bitmap;
unsigned int* vram_buffer;
int w, h, i, j, s = (20);
float xpos = 2;
int y_shift = 0;
int height = 0;
int width = 0;
int b;
// get the base type
#include "stb_textedit.h"

// define our editor structure
typedef struct {
  char* string;
  int stringlen;
  STB_TexteditState state;
  int alloc_size;
} text_control;

// define the functions we need
void layout_func(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i) {
  if (str->string[start_i] == '\n' && start_i != 0) {
    if (str->string[start_i - 1] != '\n') {
      // printf("++\n");
      start_i++;
    }
  }
  row->num_chars = start_i;
  for (; row->num_chars < str->stringlen && str->string[row->num_chars] != '\n';
       row->num_chars++) {
  }
  if (str->string[row->num_chars] == '\n') {
    row->num_chars++;
  }
  row->num_chars -= start_i;
  row->x0 = 0;                     // 横轴起始
  row->x1 = 635;                   // 横轴结束
  row->baseline_y_delta = height;  // 行高
  row->ymin = -height / 2;         // 纵轴起始
  row->ymax = height / 2;          // 纵轴结束
}
void delete_char(char* str, int pos) {
  int i;
  int l = strlen(str);
  if (l == 0) {
    return;
  }
  for (i = pos; i < strlen(str); i++) {
    str[i] = str[i + 1];
  }
  str[l - 1] = 0;
}
int delete_chars(STB_TEXTEDIT_STRING* str, int pos, int num) {
  for (int i = 0; i < num; i++) {
    delete_char(str->string, pos--);
  }
  str->stringlen -= num;
  return 1;  // always succeeds
}
void insert_char(char* str, int pos, char ch) {
  int i;
  for (i = strlen(str); i >= pos; i--) {
    str[i + 1] = str[i];
  }
  str[pos] = ch;
}
int insert_chars(STB_TEXTEDIT_STRING* str,
                 int pos,
                 STB_TEXTEDIT_CHARTYPE* newtext,
                 int num) {
  if (str->alloc_size > str->stringlen + num) {
    str->string = realloc(str->string, str->stringlen + num + 100);
    str->alloc_size = str->stringlen + num + 100;
  }
  for (int i = 0; i < num; i++) {
    insert_char(str->string, pos++, newtext[i]);
  }
  str->stringlen += num;
  return 1;  // always succeeds
}
float get_width(STB_TEXTEDIT_STRING* str, int n, int i) {
  // 返回字符的宽度
  // 这里这么多参数是为了考虑不等宽字体的，我们用不了这么多，只需要返回一个定值就行（我们用的是点阵字体）
  return width;
}
// define all the #defines needed
#define STB_TEXTEDIT_STRINGLEN(tc) ((tc)->stringlen)
#define STB_TEXTEDIT_LAYOUTROW layout_func
#define STB_TEXTEDIT_GETWIDTH get_width  // quick hack for monospaced
#define STB_TEXTEDIT_KEYTOTEXT(key) ((key))
#define STB_TEXTEDIT_GETCHAR(tc, i) ((tc)->string[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isspace(ch)
#define STB_TEXTEDIT_DELETECHARS delete_chars
#define STB_TEXTEDIT_INSERTCHARS insert_chars

#define STB_TEXTEDIT_K_LEFT (-3)
#define STB_TEXTEDIT_K_RIGHT (-4)
#define STB_TEXTEDIT_K_UP (-1)
#define STB_TEXTEDIT_K_DOWN (-2)
#define STB_TEXTEDIT_K_BACKSPACE ('\b')

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

char* ttf_buffer;
#define argb(a, r, g, b) ((a) << 24 | (r) << 16 | (g) << 8 | (b))
uint32_t LCD_AlphaBlend(uint32_t foreground_color,
                        uint32_t background_color,
                        uint8_t alpha) {
  uint16_t r = 0, g = 0, b = 0;
  if ((foreground_color == 0xffffff) &&
      (background_color == 0)) {  //默认的前景和背景色，不做alpha计算
    r = alpha;
    g = alpha;
    b = alpha;
  } else {
    uint8_t* fg = (uint8_t*)&foreground_color;
    uint8_t* bg = (uint8_t*)&background_color;

    b = ((int)(*fg * alpha) + (int)*bg * (256 - alpha)) >> 8;
    fg++;
    bg++;
    g = ((int)(*fg * alpha) + (int)*bg * (256 - alpha)) >> 8;
    fg++;
    bg++;
    r = ((int)(*fg * alpha) + (int)*bg * (256 - alpha)) >> 8;
  }
  return argb(0, r, g, b);
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
void TTF_Draw_Char(window_t wid,
                   int x,
                   int y,
                   int ch,
                   unsigned int bc,
                   unsigned fc) {
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
  GSetBuffer(((int)x0 + x), (b + y0) + y, w, h, buf1, wid);
  free(buf1);
  free(bitmap);
}
void dummy3(void) {
  stb_textedit_click(0, 0, 0, 0);
  stb_textedit_drag(0, 0, 0, 0);
  stb_textedit_cut(0, 0);
  stb_textedit_key(0, 0, 0);
  stb_textedit_initialize_state(0, 0);
  stb_textedit_paste(0, 0, 0, 0);
}
text_control obj;
window_t mainWindow;
void show() {
  SDraw_Box(mainWindow, 5, 28, 640, 480, 0xffffff, 640 + 5);
  int ys = 0;
  int xs = 0;
  int cx = 0, cy = 0;
  for (int i = 0; i <= obj.stringlen; i++) {
    if (i == obj.state.cursor) {
      cx = xs;
      cy = ys;
    }
    if (i != obj.stringlen) {
      if (xs > (640 - 5)) {
        ys += height;
        xs = 0;
      }
      if (obj.string[i] == '\n') {
        ys += height;
        xs = 0;
      } else {
        // SDraw_Char(mainWindow, 5 + xs, 28 + ys, obj.string[i], 0x0, 640 + 5);
        TTF_Draw_Char(mainWindow, 5 + xs, 28 + ys, obj.string[i], 0xffffff,
                      0x0);

        xs += width;
      }
    }
  }
  SDraw_Box(mainWindow, 5 + xs, 28 + ys, 640 - (5 + xs), 480 - (28 + ys),
            0xffffff, 640 + 5);
  SDraw_Box(mainWindow, 5 + cx, 28 + cy, 5 + cx, 28 + cy + height, 0x0,
            640 + 5);
}
int mLine(char* buffer, int len) {
  int l = 0;
  for (int i = 0; i < len; i++) {
    if (buffer[i] == '\n') {
      l++;
    }
  }
  // printf("Mline = %d\n",l);
  return l;
}
void unix_to_dos(const char* unix_str, int len, char* dos_str) {
  int i, j;
  for (i = 0, j = 0; i < len; i++, j++) {
    if (unix_str[i] == '\n') {
      dos_str[j] = '\r';
      j++;
    }
    dos_str[j] = unix_str[i];
  }
}

int PowerMain(int argc, char** argv) {
  if (argc == 1) {
    return 0;
  }
  ttf_buffer = malloc(filesize("font.ttf"));
  api_ReadFile("font.ttf", ttf_buffer);
  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));
  set_size(16);
  obj.string = malloc(100);
  obj.stringlen = 0;
  obj.alloc_size = 100;
  stb_textedit_initialize_state(&obj.state, 0);

  // stb_textedit_paste(&obj, &obj.state, "hello", 5);
  // stb_textedit_click(&obj, &obj.state, 24, 0);

  // stb_textedit_key(&obj, &obj.state, '\n');
  // stb_textedit_click(&obj, &obj.state, 0, 16);
  // stb_textedit_paste(&obj, &obj.state, "world", 5);
  // stb_textedit_key(&obj, &obj.state, STB_TEXTEDIT_K_UP);
  // stb_textedit_key(&obj, &obj.state, STB_TEXTEDIT_K_DOWN);
  // stb_textedit_paste(&obj, &obj.state, "ttttt", 5);
  // obj.string[obj.stringlen] = 0;
  // printf("%s\n", obj.string);

  mainWindow = create_window(50, 50, 640 + 5, 480 + 28, "tedit");
  // PrintChineseStr0(mainWindow, "你好世界!", 0, 28+32, 0x0, 200);
  SDraw_Box(mainWindow, 5, 28, 640, 480, 0xffffff, 640 + 5);
  show();
  while (1) {
    int sz_win = MessageLength(WINDOW_CLOSE_MSG_ID);
    int sz_ms = MessageLength(-3);
    if (sz_win == 4) {
      window_t id;
      GetMessage(&id, WINDOW_CLOSE_MSG_ID);
      if (id == mainWindow) {
        FILE* fp = fopen(argv[1], "w");
        fwrite(obj.string, obj.stringlen, 1, fp);
        // printf("%s\n",str);
        fclose(fp);
        close_window(mainWindow);
        return 0;
      }
    } else if (sz_ms == 4) {
      unsigned int id;
      GetMessage(&id, -3);
      int x = (id >> 16) - 5;
      int y = (id & 0xffff) - 28;
      if (x < 0 || y < 0) {
        x = 0;
        y = 0;
      }
      // printf("x=%d,y=%d\n",(int)x / 8 * 8,(int)y / 16 * 16);
      stb_textedit_click(&obj, &obj.state, x, y);
      show();
    }
    if (_kbhit()) {
      int c = getch();
      stb_textedit_key(&obj, &obj.state, c);
      show();
    }
  }
}