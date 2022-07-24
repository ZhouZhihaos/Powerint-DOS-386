#include <io.h>
extern struct SHEET* sht_win;   // console窗口的sheet
extern struct SHEET* sht_win_;
extern struct SHEET* sht_b_cur;
extern vram_t* buf_win;  // console窗口的缓冲区
int x, y;
int cons_x, cons_y;
unsigned int s_ne_t;
int Raw_x = 0;
int Raw_y = 0;
void clear() {
  if (running_mode == POWERDESKTOP) {
    boxfill(buf_win, CMDLINEXSIZE, COL_000000, 5, 24, CMDLINEXSIZE - 5,
             CMDLINEYSIZE - 5);
    sheet_refresh(sht_win_, 5, 24, CMDLINEXSIZE - 5, CMDLINEYSIZE - 5);
    sheet_refresh(sht_win,5, 24, CMDLINEXSIZE - 5, CMDLINEYSIZE - 5);
  }
  int i;
  int j;
  for (i = 0; i < 160; i += 2) {
    for (j = 0; j < 25; j++) {
      *(char*)(0xb8000 + j * 160 + i) = ' ';
      *(char*)(0xb8000 + j * 160 + i + 1) = get_cons_color();
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
int getlength(const char* str) {
  int i;
  for (i = 0;; ++i) {
    if (str[i] == '\0') {
      return i;
    }
  }
}
void putstr(const char* str, int length) {
  io_cli();
  int i;
  for (i = 0; i < length; i++) {
    if (y == 24 && x >= 160) {
      /*暂时什么也不做*/
      // TODO:屏幕滚动！！！！
      screen_ne();
    }
    if (str[i] == 0x0d) {
      continue;
    }
    putchar(str[i]);
    io_cli();
  }
  io_sti();
}
void init_s_ne_t() {
  s_ne_t = 0;
}
int Get_S_NE_T() {
  return s_ne_t;
}
void printchar(char ch) {
  char ch1[2] = {ch, '\0'};
  print(ch1);
}
void outputchar_b800(int x, int y, char ch) {
  x *= 2;
  *(char*)(0xb8000 + y * 160 + x) = ch;
}
void putchar(char ch) {
  io_cli();
  if (x == 160) {
    y++;
    Raw_y++;
    cons_y++;
    x = 0;
    cons_x = 0;
    Raw_x = 0;

    Move_Cursor(cons_x, cons_y);
  }
  if (ch == '\n') {
    if (y == 24) {
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
  } else if (ch == '\0') {
    // gotoxy(get_x()+1,get_y());
    io_sti();
    return;
  } else if (ch == '\b') {
    if (x == 0) {
      cons_y--;
      y--;
      cons_x = 79;
      Raw_y--;
      Raw_x = 158;
      x = 158;
      Move_Cursor(cons_x, cons_y);
      *(char*)(0xb8000 + y * 160 + x) = ' ';
      *(char*)(0xb8000 + y * 160 + x - 2 + 1) = get_cons_color();
      io_sti();
      return;
    }
    *(char*)(0xb8000 + y * 160 + x - 2) = ' '; /*显示位置为第23行*/
    *(char*)(0xb8000 + y * 160 + x - 2 + 1) = get_cons_color();
    x -= 2;
    cons_x--;
    Move_Cursor(cons_x, cons_y);
    io_sti();
    return;
  } else if (ch == '\t') {
    //制表符
    print("    ");
    io_sti();
    return;
  }
  cons_x += 1;
  Move_Cursor(cons_x, cons_y);
  *(char*)(0xb8000 + y * 160 + x) = ch; /*显示位置为第23行*/
  *(char*)(0xb8000 + y * 160 + x + 1) = get_cons_color();
  Raw_x += 2;

  x += 2;
  io_sti();
}
void Move_Cursor(short x, short y) {
  if(running_mode == POWERDESKTOP) {
   // sheet_updown(sht_b_cur,1);
    int ox = sht_b_cur->vx0;
    int oy = sht_b_cur->vy0;
    sheet_slide(sht_b_cur, x*8+5, y*16+24);
    sheet_refresh(sht_win,ox,oy,ox+8,oy+16);
    sheet_refresh(sht_win,(x*8+5),(y*16+24),(x*8+5)+8,(y*16+24)+16);
    return;
  }
  int res = y * 80 + x;
  ASM_call(res);
}
void uint32_memcpy(unsigned int* dest, unsigned int* src, int length) {
  int i;
  for (i = 0; i < length; i++) {
    dest[i] = src[i];
  }
}
vram_t* vramcpy(vram_t* s, const vram_t* ct, size_t n) {
  if (NULL == s || NULL == ct || n <= 0)
    return NULL;
  while (n--)
    *(vram_t*)s++ = *(vram_t*)ct++;
  return s;
}
void screen_ne_cons()
{
  if (running_mode == POWERDESKTOP) {
    for (int j = 0; j != 16; j++) {
      for (int i = 0; i != 25 * 16; i++) {
        vramcpy(buf_win + (CMDLINEXSIZE * (24 + i)) + 5,
               buf_win + (CMDLINEXSIZE * (24 + i + 1)) + 5, CMDLINEXSIZE - 10);
      }
    }
    // for (int i = 0; i != 16; i++) {
    //   memset(buf_win + (CMDLINEXSIZE * (CMDLINEYSIZE - 5 - 16 + i)) + 5, 0,
    //          CMDLINEXSIZE - 10);
    // }
    sheet_refresh(sht_win_, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
    sheet_refresh(sht_win, 0, 0, CMDLINEXSIZE, CMDLINEYSIZE);
  }
}
void screen_ne() /*向下滚动一行*/
{
  // TODO:屏幕滚动！！！！
  int i;
  int j;
  int X = get_x();
  for (i = 0; i < 160; i += 2) {
    for (j = 0; j < 24; j++) {
      *(char*)(0xb8000 + j * 160 + i) = *(char*)(0xb8000 + (j + 1) * 160 + i);
      *(char*)(0xb8000 + j * 160 + i + 1) =
          *(char*)(0xb8000 + (j + 1) * 160 + i + 1);
    }
  }
  for (i = 0; i < 160; i += 2) {
    *(char*)(0xb8000 + 24 * 160 + i) = ' ';
    *(char*)(0xb8000 + 24 * 160 + i + 1) = get_cons_color();
  }

  cons_x = 0;
  x = 0;
  s_ne_t++;
  Raw_y++;
  Raw_x = 0;
  Move_Cursor(cons_x, cons_y);
}
int get_x() {
  return cons_x;
}
int get_y() {
  return cons_y;
}
int Get_Raw_x() {
  return Raw_x;
}
int Get_Raw_y() {
  return Raw_y;
}
void print(const char* str) {
  if (running_mode == POWERDESKTOP) {
    // 在console窗口打印 按照cons_x cons_y
    io_cli();
    putfonts_asc_cons(buf_win, CMDLINEXSIZE, 5 + cons_x * 8, 24 + cons_y * 16,
                       COL_FFFFFF, str);
    // sheet_refresh(sht_win, 5 + cons_x * 8 - 8, 24 + cons_y * 16,
    //               5 + cons_x * 8 + getlength(str) * 8, 24 + cons_y * 16 + 16);
  }
  putstr(str, getlength(str));
}

void GotoXy_No_Safe(int x1, int y1) {
  cons_x = x1;
  cons_y = y1;
  x = x1 * 2;
  y = y1;
  Move_Cursor(cons_x, cons_y);
}
void gotoxy(int x1, int y1) {
  io_cli();
  if (x1 >= 0 && y1 >= 0) {
    int x2 = x1;
    int y2 = y1;
    if (x1 <= 79 && y1 <= 24) {
      GotoXy_No_Safe(x1, y1);
      io_sti();
      return;
    }
    if (x1 <= 79) {
      for (int i = 0; i < y1 - 24 - 1; i++) {
        screen_ne();
      }
      GotoXy_No_Safe(x1, 24);
      io_sti();
      return;
    }

    if (x1 > 79) {
      y2 += x1 / 80 - 1;
      x2 = x1 % 80;
      if (y2 <= 24)
        gotoxy(x2, y2 + 1);
      else
        gotoxy(x2, y2);
    }
  } else {  //负数
    //如果x1为负数 说明y1要--
    if (x1 < 0) {
      x1 += 80;
      y1--;
      gotoxy(x1, y1);
    }
    if (y1 < 0) {
      io_sti();
      return;
    }
  }
  io_sti();
}
// 截图
void screen_shot() {
  unsigned char* buf = (unsigned char*)malloc(80 * 8 * 25 * 16 * 3);
  int x = 0, y = 0;
  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 160; j += 2) {
      char ch = *(char*)(0xb8000 + i * 160 + j);
      Draw_Char_32(buf, 80 * 8, ch, x, y, 0xff, 0xff, 0xff);
      x += 8;
    }
    y += 16;
  }
  MakePraFile("abc.pra", buf, 80 * 8, 25 * 16);
}
int HighPrintf(const char* format, ...) {
  int len;
  va_list ap;
  va_start(ap, format);
  char buf[1024];
  len = vsprintf(buf, format, ap);
  HighPrint(buf);
  va_end(ap);
  return len;
}