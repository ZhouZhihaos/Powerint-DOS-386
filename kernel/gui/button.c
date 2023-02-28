#include <dos.h>
#include <gui.h>
extern struct SHTCTL *shtctl;
extern List *list;
extern List *list2;
bool Collision(int x, int y, int w, int h, int x1, int y1) {
  // 矩形碰撞检测
  // x,y是矩形坐标
  // w,h是矩形宽高
  // x1,y1是需要检测的点
  if (x1 <= x + w && x1 >= x && y1 <= y + h && y1 >= y) {
    return true;
  }
  return false;
}
bool Button_Click(Button *buf, int _x, int _y) {
  // 判断是否点击到按钮上
  int x = buf->x;
  int y = buf->y;
  int w = buf->w;
  int h = buf->h;
  extern struct SHEET *sht_back;
  if (buf->buf != sht_back && !(buf->buf->height >= shtctl->top - 1)) {
    return false;
  }
  return Collision(buf->buf->vx0 + x, buf->buf->vy0 + y, w, h, _x, _y);
}
void DeleteButton(Button *buf) {
  if (buf->cont != -1) {
    for (int i = 1; FindForCount(i, list) != NULL; i++) {
      if (FindForCount(i, list)->val == (int)buf) {
        DeleteVal(i, list);
        free(buf->text);
        page_free(buf, sizeof(Button));
        break;
      }
    }
  }
  return;
}
void DeleteButton_NoFrame(Button *buf) {
  if (buf->cont != -1) {
    for (int i = 1; FindForCount(i, list2) != NULL; i++) {
      if (FindForCount(i, list2)->val == (int)buf) {
        DeleteVal(i, list2);
        free(buf->text);
        page_free(buf, sizeof(Button));
        break;
      }
    }
  }
  return;
}
void Button_Draw(Button *buf) {
  int x = buf->x;
  int y = buf->y;
  int w = buf->w;
  int h = buf->h;
  int dy = 0; // 按钮的文字垂直偏移量
  if (h > 20) {
    dy = (h - 20) / 2;
  }

  boxfill(buf->buf->buf, buf->buf->bxsize, COL_C6C6C6, x, y, x + w, y + h);
  putfonts_asc_sht(buf->buf, x + (w - strlen(buf->text) * 8) / 2, y + dy,
                   COL_000000, COL_C6C6C6, buf->text, strlen(buf->text));
  Draw_Line(buf->buf->buf, x + w - 1, y, x + w - 1, y + h, COL_848484,
            buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x + w, y, x + w, y + h, COL_000000,
            buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y, x + w, y, COL_FFFFFF, buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y, x, y + h, COL_FFFFFF, buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y + h, x + w, y + h, COL_000000,
            buf->buf->bxsize);
  sheet_refresh(buf->buf, 0, 0, buf->buf->bxsize, buf->buf->bysize);
}
void Button_Draw_Clicking(Button *buf) {
  int x = buf->x;
  int y = buf->y;
  int w = buf->w;
  int h = buf->h;
  int dy = 0; // 按钮的文字垂直偏移量
  if (h > 20) {
    dy = (h - 20) / 2;
  }

  boxfill(buf->buf->buf, buf->buf->bxsize, COL_C6C6C6, x, y, x + w, y + h);
  putfonts_asc_sht(buf->buf, x + (w - strlen(buf->text) * 8) / 2, y + dy,
                   COL_000000, COL_C6C6C6, buf->text, strlen(buf->text));
  // 按钮被按下是凹陷状
  Draw_Line(buf->buf->buf, x + w - 1, y, x + w - 1, y + h, COL_000000,
            buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x + w, y, x + w, y + h, COL_000000,
            buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y, x + w, y, COL_000000, buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y, x, y + h, COL_000000, buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x, y + h, x + w, y + h, COL_FFFFFF,
            buf->buf->bxsize);
  Draw_Line(buf->buf->buf, x + w, y + h, x + w, y, COL_FFFFFF,
            buf->buf->bxsize);
  sheet_refresh(buf->buf, 0, 0, buf->buf->bxsize, buf->buf->bysize);
}
Button *MakeButton(int x, int y, int w, int h, struct SHEET *buf, char *text,
                   void (*OnClick)()) {
  uint8_t *button_text_buffer = (uint8_t *)malloc(strlen(text) + 1);
  strcpy(button_text_buffer, text);
  Button *buf_ = (Button *)page_malloc(sizeof(Button));
  buf_->x = x + 3;
  buf_->y = y + 28;
  buf_->w = w;
  buf_->h = h;
  buf_->buf = buf;
  buf_->text = button_text_buffer;
  buf_->OnClick = OnClick;
  buf_->is_clicking = false;
  buf_->cont = -1;
  buf_->task = NowTask();
  Button_Draw(buf_);
  AddVal((int)buf_, list);
  buf_->cont = GetLastCount(list);
  return buf_;
}
void Button_Draw_NoFrame(Button *btn) {
  if (btn->hide) return;
  SDraw_Box(btn->buf->buf, btn->x, btn->y, btn->x + btn->w, btn->y + btn->h,
            btn->bc, btn->buf->bxsize);
  int dy = 0; // 按钮的文字垂直偏移量
  if (btn->h > 20) {
    dy = (btn->h - 20) / 2;
  }
  Sputs(btn->buf->buf, btn->text, btn->x + (btn->w - strlen(btn->text) * 8) / 2,
        btn->y + dy, COL_000000, btn->buf->bxsize);
  sheet_refresh(btn->buf, btn->x, btn->y, btn->x + btn->w + 1, btn->y + btn->h + 1);
}
void Button_Draw_NoFrame_Clicking(Button *btn) {
  if (btn->hide) return;
  SDraw_Box(btn->buf->buf, btn->x, btn->y, btn->x + btn->w, btn->y + btn->h,
            argb(0, 10, 36, 106), btn->buf->bxsize);
  int dy = 0; // 按钮的文字垂直偏移量
  if (btn->h > 20) {
    dy = (btn->h - 20) / 2;
  }
  Sputs(btn->buf->buf, btn->text, btn->x + (btn->w - strlen(btn->text) * 8) / 2,
        btn->y + dy, COL_FFFFFF, btn->buf->bxsize);
  sheet_refresh(btn->buf, btn->x, btn->y, btn->x + btn->w + 1, btn->y + btn->h + 1);
}
Button *MakeButton_NoFrame(int x, int y, int w, int h, struct SHEET *buf,
                           char *text, void (*OnClick)(), color_t bc, bool hide) {
  uint8_t *button_text_buffer = (uint8_t *)malloc(strlen(text) + 1);
  strcpy(button_text_buffer, text);
  Button *buf_ = (Button *)page_malloc(sizeof(Button));
  buf_->x = x;
  buf_->y = y;
  buf_->w = w;
  buf_->h = h;
  buf_->buf = buf;
  buf_->text = button_text_buffer;
  buf_->OnClick = OnClick;
  buf_->is_clicking = false;
  buf_->cont = -1;
  buf_->task = NowTask();
  buf_->bc = bc;
  buf_->hide = hide;
  Button_Draw_NoFrame(buf_);
  AddVal((int)buf_, list2);
  buf_->cont = GetLastCount(list2);
  return buf_;
}
