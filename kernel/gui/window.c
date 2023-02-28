#include <dos.h>
#include <gui.h>
extern struct SHTCTL *shtctl;
extern List *list;
extern List *list1;
struct SHEET *MakeWindow(int x, int y, int w, int h, char *title,
                         struct SHTCTL *stl, vram_t *vbuf, void (*Close)()) {
  extern struct VBEINFO *vbinfo;
  struct SHEET *buf;
  buf = sheet_alloc(stl);
  buf->Close = Close;
  buf->task = NowTask();
  sheet_setbuf(buf, vbuf, w, h, -1);
  make_window(vbuf, w, h, title);
  struct SHEET *old = stl->sheets[stl->top - 1];
  uint32_t times = (old->bxsize - 8) / (0xc6 - 0x84) + 1;
  for (int i = 3; i < 20; i++) {
    color_t color = COL_848484;
    for (int j = 3, count = 0; j < old->bxsize - 4; j++, count++) {
      if (old->buf[j + i * old->bxsize] != COL_FFFFFF &&
          !(old->bxsize - 37 <= j && j < old->bxsize - 5 && 5 <= i && i < 19)) {
        old->buf[j + i * old->bxsize] = color;
      }
      if (count == times && color != COL_C6C6C6) {
        color += 0x00010101;
        count = 0;
      }
    }
  }
  sheet_refresh(old, 3, 3, old->bxsize - 4, 20);
  sheet_updown(buf, stl->top);
  sheet_slide(buf, x, y);
  for (int L = 1; GetTask(L) != NULL; L++) {
    if (GetTask(L) == NowTask()) {
      // SubTask(L);
      // break;
      continue;
    }
    SleepTaskFIFO(GetTask(L));
  }
  extern struct SHEET *sht;
  sht = buf;
  return buf;
}
void make_window(vram_t *buf, int xsize, int ysize, char *title) {
  static char *closebtn[14] = {
      "OOOOOOOOOOOOOOO@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@",
      "OQQQ@@QQQQ@@QQ$@", "OQQQQ@@QQ@@QQQ$@", "OQQQQQ@@@@QQQQ$@",
      "OQQQQQQ@@QQQQQ$@", "OQQQQQ@@@@QQQQ$@", "OQQQQ@@QQ@@QQQ$@",
      "OQQQ@@QQQQ@@QQ$@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@",
      "O$$$$$$$$$$$$$$@", "@@@@@@@@@@@@@@@@"};
  static char *smallbtn[14] = {
      "OOOOOOOOOOOOOOO@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@",
      "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@",
      "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@",
      "OQQ@@@@@@@@@QQ$@", "OQQ@@@@@@@@@QQ$@", "OQQQQQQQQQQQQQ$@",
      "O$$$$$$$$$$$$$$@", "@@@@@@@@@@@@@@@@",
  };
  int x, y;
  color_t c;
  boxfill(buf, xsize, COL_C6C6C6, 0, 0, xsize - 1, 0);
  boxfill(buf, xsize, COL_FFFFFF, 1, 1, xsize - 2, 1);
  boxfill(buf, xsize, COL_C6C6C6, 0, 0, 0, ysize - 1);
  boxfill(buf, xsize, COL_FFFFFF, 1, 1, 1, ysize - 2);
  boxfill(buf, xsize, COL_848484, xsize - 2, 1, xsize - 2, ysize - 2);
  boxfill(buf, xsize, COL_000000, xsize - 1, 0, xsize - 1, ysize - 1);
  boxfill(buf, xsize, COL_C6C6C6, 2, 2, xsize - 3, ysize - 3);

  // boxfill(buf, xsize, COL_000084, 3, 3, xsize - 4, 20);
  uint32_t times = (xsize - 8) / (255 - 106) + 1;
  for (int i = 3; i < 20; i++) {
    color_t color = argb(0, 10, 36, 106);
    for (int j = 3, count = 0; j < xsize - 4; j++, count++) {
      buf[j + i * xsize] = color;
      if (count == times && (color & 0xff) != 255) {
        color += 0x00010101;
        count = 0;
      }
    }
  }

  boxfill(buf, xsize, COL_848484, 1, ysize - 2, xsize - 2, ysize - 2);
  boxfill(buf, xsize, COL_000000, 0, ysize - 1, xsize - 1, ysize - 1);
  putfonts_asc(buf, xsize, 24, 4, COL_FFFFFF, (unsigned char *)title);

  for (y = 0; y < 14; y++) {
    for (x = 0; x < 16; x++) {
      if (closebtn[y][x] == '@') {
        c = COL_000000;
      } else if (closebtn[y][x] == '$') {
        c = COL_848484;
      } else if (closebtn[y][x] == 'Q') {
        c = COL_C6C6C6;
      } else {
        c = COL_FFFFFF;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
    }
  }
  for (y = 0; y < 14; y++) {
    for (x = 0; x < 16; x++) {
      if (smallbtn[y][x] == '@') {
        c = COL_000000;
      } else if (smallbtn[y][x] == '$') {
        c = COL_848484;
      } else if (smallbtn[y][x] == 'Q') {
        c = COL_C6C6C6;
      } else {
        c = COL_FFFFFF;
      }
      buf[(5 + y) * xsize + (xsize - 37 + x)] = c;
    }
  }
  return;
}
void make_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
  int x1 = x0 + sx, y1 = y0 + sy;
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
  boxfill(sht->buf, sht->bxsize, COL_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
  boxfill(sht->buf, sht->bxsize, COL_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
  boxfill(sht->buf, sht->bxsize, COL_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
  boxfill(sht->buf, sht->bxsize, COL_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
  boxfill(sht->buf, sht->bxsize, COL_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
  boxfill(sht->buf, sht->bxsize, COL_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
  boxfill(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
  Draw_text_box_Cur_sht(sht, x0, y0);
  return;
}
TextBox *MakeTextBox(int x, int y, int w, int h, struct SHEET *window_sht) {
  TextBox *buf = (TextBox *)page_malloc(sizeof(TextBox));
  // x += 3;
  // y += 28;
  // 使y处于TextBox的中间
  buf->x = x;
  buf->y = y;
  buf->w = w;
  buf->h = h;
  buf->buf = window_sht;
  buf->text =
      (char *)page_malloc(sizeof(char) * w / 8 + 1); // 多出的一个字节是\0
  buf->text[0] = '\0';
  buf->is_clicking = false;
  buf->Write_Pos_X = 0;
  // buf->Write_Pos_Y = 0;
  make_textbox(window_sht, x, y, w, h, COL_FFFFFF);
  sheet_refresh(window_sht, 0, 0, window_sht->bxsize, window_sht->bysize);
  AddVal((int)buf, list1);
  // buf->y += h/ 2;
  return buf;
}
bool TextBox_Click(TextBox *buf, int _x, int _y) {
  int x = buf->x;
  int y = buf->y;
  int w = buf->w;
  int h = buf->h;
  if (!(buf->buf->height >= shtctl->top - 1)) {
    return false;
  }
  return Collision(buf->buf->vx0 + x, buf->buf->vy0 + y, w, h, _x, _y);
}
void TextBox_AddChar(TextBox *buf, char ch) {
  if (buf->Write_Pos_X / 8 >= buf->w / 8 && ch != '\b' || ch == '\n') {
    return;
  }
  if (ch == '\b') {
    if (buf->Write_Pos_X == 0) {
      return;
    }
    if (buf->Write_Pos_X / 8 < buf->w / 8) {
      putfonts_asc_sht(buf->buf, buf->x + buf->Write_Pos_X, buf->y, COL_000000,
                       COL_FFFFFF, " ", 1);
    }
    buf->Write_Pos_X -= 8;
    buf->text[buf->Write_Pos_X / 8] = 0;
    putfonts_asc_sht(buf->buf, buf->x + buf->Write_Pos_X, buf->y, COL_000000,
                     COL_FFFFFF, " ", 1);
    Draw_text_box_Cur_sht(buf->buf, buf->x + buf->Write_Pos_X, buf->y);
    return;
  }
  char chs[2] = {ch, 0};
  buf->text[buf->Write_Pos_X / 8] = ch;
  buf->text[buf->Write_Pos_X / 8 + 1] = 0;
  putfonts_asc_sht(buf->buf, buf->x + buf->Write_Pos_X, buf->y, COL_000000,
                   COL_FFFFFF, chs, 1);
  buf->Write_Pos_X += 8;
  if (buf->Write_Pos_X / 8 < buf->w / 8) {
    // 画光标
    Draw_text_box_Cur_sht(buf->buf, buf->x + buf->Write_Pos_X, buf->y);
  }
}
void Draw_text_box_Cur_sht(struct SHEET *buf, int x, int y) {
  // 画一个8*16的黑色矩形
  boxfill(buf->buf, buf->bxsize, COL_000000, x, y, x + 8 - 1, y + 16 - 1);
  sheet_refresh(buf, x, y, x + 8 - 1, y + 16 - 1);
}
void Draw_Cur(vram_t *vram, int x, int y, int xsize) {
  static char *CUR[16] = {"........", "........", "........", "........",
                          "........", "........", "........", "........",
                          "........", "........", "........", "........",
                          "........", "........", "********", "********"};
  int i, j;
  for (i = 0; i < 16; i++) {
    for (j = 0; j < 8; j++) {
      if (CUR[i][j] == '.') {
        vram[(y + i) * xsize + x + j] = COL_TRANSPARENT;
      } else if (CUR[i][j] == '*') {
        vram[(y + i) * xsize + x + j] = COL_FFFFFF;
      }
    }
  }
}
void MsgBoxClose() {
  struct SHEET *_sht = shtctl->sheets[shtctl->top - 1];
  for (int i = 1; FindForCount(i, list) != NULL; i++) {
    Button *btn = (Button *)FindForCount(i, list)->val;
    if (btn->buf == _sht) {
      DeleteButton(btn);
      page_free(_sht->buf, _sht->bxsize * _sht->bysize * sizeof(color_t));
      sheet_free(_sht);
      break;
    }
  }
}
void MsgBox(char *infomation, char *title) {
  extern struct VBEINFO *vbinfo;
  int w = strlen(infomation) * 8 + 16 + 170;
  int h = 125;
  vram_t *Msg_Buf = (vram_t *)page_malloc(w * h * sizeof(color_t));
  struct SHEET *msg_box = MakeWindow(vbinfo->xsize / 2, vbinfo->ysize / 2, w, h,
                                     title, shtctl, Msg_Buf, NULL);
  int x = (w - strlen(infomation) * 8) / 2;
  int y = (h - 16) / 2;
  putfonts_asc_sht(msg_box, x, y, COL_000000, COL_C6C6C6, infomation,
                   strlen(infomation));
  Button *button = MakeButton(
      // xy(窗口下方)
      w / 2 - 28, h - 34 - 28,
      // width
      64,
      // height
      32, msg_box, "OK", MsgBoxClose);
}
void DeleteTextBox(TextBox *buf) {
  for (int i = 1; FindForCount(i, list1) != NULL; i++) {
    if (FindForCount(i, list1)->val == (int)buf) {
      DeleteVal(i, list1);
      free(buf->text);
      page_free(buf, sizeof(TextBox));
      break;
    }
  }

  return;
}
