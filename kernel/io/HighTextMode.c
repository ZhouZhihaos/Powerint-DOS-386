#include <io.h>
static int bx = 0, by = 0;
static int x = 0, y = 0;
static struct VBEINFO* vinfo = (struct VBEINFO*)VBEINFO_ADDRESS;
static vram_t* scr_buf;
static vram_t* cur_buf;
static struct SHEET *sht_scr, *sht_cur;
static struct SHTCTL* shtctl0;
static struct FIFO8 fifo;
static int cur_x = 0, cur_y = 0;
extern int Raw_x, Raw_y;
void put_sht(struct SHEET* sht, int x, int y, int l) {
  // boxfill(sht->buf, sht->bxsize, 0, x, y, x + l * 8 - 1, y + 15);
  sheet_refresh(sht, x, y, x + l * 8, y + 16);
  return;
}
void High_clear(void) {
  //高分辨率模式的清屏
  memset(scr_buf, 0, vinfo->xsize * vinfo->ysize * sizeof(color_t));
  sheet_refresh(sht_scr, 0, 0, vinfo->xsize, vinfo->ysize);
  bx = 0;
  by = 0;
  cur_x = 0;
  cur_y = 0;
}
void High_Screen_ne(void) {
  //高分辨率模式下的屏幕滚动(向下,每次移动一行(8*16))
  cur_x = 0;
  cur_y = vinfo->ysize - 16;
  memcpy(scr_buf, scr_buf + vinfo->xsize * 16 * sizeof(color_t),
         vinfo->xsize * (vinfo->ysize - 16) * sizeof(color_t));
  memset(scr_buf + vinfo->xsize * (vinfo->ysize - 16) * sizeof(color_t), 0, vinfo->xsize * 16 * sizeof(color_t));
  sheet_refresh(sht_scr, 0, 0, vinfo->xsize, vinfo->ysize);
  bx = 0;
  by = vinfo->ysize / 16 - 1;
}
void HighPrint(char* string) {
  char* p = string;
  int i;
  int length = strlen(string);
  struct VBEINFO* vinfo = VBEINFO_ADDRESS;
  if (vinfo->vram == 0) {
    return;
  }
  for (i = 0; i < length; i++) {
    if (p[i] == '\r') {
      continue;
    }
    if (by == vinfo->ysize / 16 - 1 && bx == vinfo->xsize / 8) {
      High_Screen_ne();
    }
    if (bx == vinfo->xsize / 8) {
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
    if (str[0] == 0x0d && str[1] == 0x0a) {
      by++;
      Raw_y++;
      bx = 0;
      i++;
      cur_y += 8;
      cur_x = 0;
      continue;
    } else if (str[0] == '\n') {
      if (by >= vinfo->ysize / 16 - 1) {
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
    } else if (str[0] == '\b') {
      if (bx > 0) {
        bx--;
        cur_x -= 8;
        int bmx = bx;
        int bmy = by;
        SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, COL_000000,
                  vinfo->xsize);
        bx++;
        cur_x += 8;
        put_sht(sht_scr, bmx * 8, bmy * 16, 1);
        bmx = bx;
        bmy = by;
        SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, COL_000000,
                  vinfo->xsize);
        bx++;
        cur_x += 8;
        put_sht(sht_scr, bmx * 8, bmy * 16, 1);
        bx -= 2;
        cur_x -= 16;
      }
      continue;
    }
    if (GetCHorEN(str) == 1) {
      int bmx = bx;
      int bmy = by;
      if (bx == vinfo->xsize / 8 - 1) {
        by++;
        bx = 0;
        Raw_x = 0;
        Raw_y++;
        cur_x = 0;
        cur_y += 16;
      }
      PUTCHINESE0(scr_buf, bx * 8, by * 16, COL_FFFFFF,
                  *(short*)str, vinfo->xsize);

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
    SDraw_Char(scr_buf, bx, by, c, COL_FFFFFF, vinfo->xsize);
    bx++;
    Raw_x++;
    put_sht(sht_scr, bmx * 8, bmy * 16, 1);
    cur_x += 8;
  }
}
void HighPutStr(char* string, int length) {
  char* p = string;
  int i;
  struct VBEINFO* vinfo = VBEINFO_ADDRESS;
  if (vinfo->vram == 0) {
    return;
  }
  for (i = 0; i < length; i++) {
    if (p[i] == '\r') {
      continue;
    }
    if (by == vinfo->ysize / 16 - 1 && bx == vinfo->xsize / 8) {
      High_Screen_ne();
    }
    if (bx == vinfo->xsize / 8) {
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
    if (str[0] == 0x0d && str[1] == 0x0a) {
      by++;
      Raw_y++;
      bx = 0;
      i++;
      cur_y += 8;
      cur_x = 0;
      continue;
    } else if (str[0] == '\n') {
      if (by >= vinfo->ysize / 16 - 1) {
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
    } else if (str[0] == '\b') {
      if (bx > 0) {
        bx--;
        cur_x -= 8;
        int bmx = bx;
        int bmy = by;
        SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, COL_000000,
                  vinfo->xsize);
        bx++;
        cur_x += 8;
        put_sht(sht_scr, bmx * 8, bmy * 16, 1);
        bmx = bx;
        bmy = by;
        SDraw_Box(scr_buf, bx * 8, by * 16, bx * 8 + 8, by * 16 + 16, COL_000000,
                  vinfo->xsize);
        bx++;
        cur_x += 8;
        put_sht(sht_scr, bmx * 8, bmy * 16, 1);
        bx -= 2;
        cur_x -= 16;
      }
      continue;
    }
    if (GetCHorEN(str) == 1) {
      int bmx = bx;
      int bmy = by;
      if (bx == vinfo->xsize / 8 - 1) {
        by++;
        bx = 0;
        Raw_x = 0;
        Raw_y++;
        cur_x = 0;
        cur_y += 16;
      }
      PUTCHINESE0(scr_buf, bx * 8, by * 16, COL_FFFFFF,
                  *(short*)str, vinfo->xsize);

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
    SDraw_Char(scr_buf, bx, by, c, COL_FFFFFF, vinfo->xsize);
    bx++;
    Raw_x++;
    put_sht(sht_scr, bmx * 8, bmy * 16, 1);
    cur_x += 8;
  }
}
void HighGotoXY(int x, int y) {
  bx = x;
  by = y;
  cur_x = x * 8;
  cur_y = y * 16;
}
void Gar_Test_Task() {
  int fifo_buf[128];
  struct FIFO8 fifo;
  fifo8_init(&fifo, 128, fifo_buf);
  struct TIMER* timer;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 250);
  int i = 0;
  while (1) {
    sheet_slide(sht_cur, cur_x, cur_y);
    if (fifo8_status(&fifo) != 0) {
      i = fifo8_get(&fifo);
      if (i == 1) {
        sheet_updown(sht_cur, -1);
        timer_init(timer, &fifo, 2);
        timer_settime(timer, 250);
      } else if (i == 2) {
        sheet_updown(sht_cur, 1);
        timer_init(timer, &fifo, 1);
        timer_settime(timer, 250);
      }
    }
  }
}
unsigned int stack;
void SwitchToHighTextMode() {
  if (set_mode(1024, 768, 32) != 0) {
    printf("Can't enable 1024x768x32 VBE mode.\n\n");
    return;
  }
  bx = 0;
  by = 0;
  cur_x = 0;
  cur_y = 0;
  int i, fifobuf[128];
  shtctl0 = shtctl_init(vinfo->vram, vinfo->xsize, vinfo->ysize);
  scr_buf = page_malloc(vinfo->xsize * vinfo->ysize * sizeof(color_t));
  cur_buf = page_malloc(135 * 75 * sizeof(color_t));
  sht_scr = sheet_alloc(shtctl0);
  sht_cur = sheet_alloc(shtctl0);
  sheet_setbuf(sht_scr, scr_buf, vinfo->xsize, vinfo->ysize, -1);
  sheet_setbuf(sht_cur, cur_buf, 8, 16, 99);
  memset(scr_buf, 0, vinfo->xsize * vinfo->ysize * sizeof(color_t));
  Draw_Cur(cur_buf, 0, 0, 8);
  sheet_slide(sht_scr, 0, 0);
  sheet_slide(sht_cur, cur_x, cur_y);
  sheet_updown(sht_scr, 0);
  sheet_updown(sht_cur, 1);
  stack = (unsigned int)page_malloc(64 * 1024);
  AddTask("t1", 1, 2 * 8, (int)Gar_Test_Task, 1 * 8, 1 * 8, stack + 64 * 1024);
  Raw_x = 0;
  Raw_y = 0;
  running_mode = 3;
}
void Close_High_Text_Mode(void) {
  SubTask(GetTaskForName("t1"));
  sheet_free(sht_scr);
  sheet_free(sht_cur);
  ctl_free(shtctl0);
  page_free(stack, 64 * 1024);
  page_free((int)scr_buf, vinfo->xsize * vinfo->ysize * sizeof(color_t));
  page_free((int)cur_buf, 135 * 75 * sizeof(color_t));
}
