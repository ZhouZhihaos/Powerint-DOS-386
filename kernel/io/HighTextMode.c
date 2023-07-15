#include <dos.h>
static struct SHEET *sht_cur;
static struct SHTCTL *shtctl0;
// static struct FIFO8 fifo;
// static struct TASK *t1;
// static unsigned int stack;
color_t color_table[16] = {COL_000000, COL_000084, COL_008400, COL_008484,
                           COL_840000, COL_840084, COL_848400, COL_C6C6C6,
                           COL_848484, COL_0000FF, COL_00FF00, COL_00FFFF,
                           COL_FF0000, COL_FF00FF, COL_FFFF00, COL_FFFFFF};
static void put_sht(struct SHEET *sht, int x, int y, int l) {
  // boxfill(sht->buf, sht->bxsize, 0, x, y, x + l * 8 - 1, y + 15);
  sheet_refresh(sht, x, y, x + l * 8, y + 16);
  return;
}
void clear_HighTextMode(struct tty *res) {
  //高分辨率模式的清屏
  io_cli();
  struct SHEET *sht = (struct SHEET *)res->vram;
  for (int i = 0; i != (res->xsize * 8 * res->ysize * 16); i++) {
    sht->buf[i] = color_table[res->color >> 4];
  }
  sheet_refresh(sht, 0, 0, res->xsize * 8, res->ysize * 16);
  res->x = 0;
  res->y = 0;
  res->MoveCursor(res, res->x, res->y);
  io_sti();
}
void screen_ne_HighTextMode(struct tty *res) {
  //高分辨率模式下的屏幕滚动(向下,每次移动一行(8*16))
  io_cli();
  struct SHEET *sht = (struct SHEET *)res->vram;
  memcpy((void *)sht->buf,
         (void *)sht->buf + res->xsize * 8 * 16 * sizeof(color_t),
         res->xsize * 8 * (res->ysize - 1) * 16 * sizeof(color_t));
  for (int i = (res->xsize * 8 * (res->ysize - 1) * 16);
       i != (res->xsize * 8 * res->ysize * 16); i++) {
    sht->buf[i] = color_table[res->color >> 4];
  }
  sheet_refresh(sht, 0, 0, res->xsize * 8, res->ysize * 16);
  res->x = 0;
  res->y = res->ysize - 1;
  res->Raw_y++;
  res->MoveCursor(res, res->x, res->y);
  io_sti();
}
void putchar_HighTextMode(struct tty *res, int c) {
  struct SHEET *sht = (struct SHEET *)res->vram;
  unsigned char *p = &c;
  if (p[0] == '\r') {
    return;
  }
  if (res->y == res->ysize - 1 && res->x == res->xsize) {
    res->screen_ne(res);
  }
  if (res->x == res->xsize) {
    res->y++;
    res->x = 0;
    res->MoveCursor(res, res->x, res->y);
  }
  unsigned char str[3];
  str[0] = p[0];
  str[1] = p[1];
  str[2] = 0;
  if (str[0] == '\n') {
    if (res->y >= res->ysize - 1) {
      res->screen_ne(res);
      return;
    }
    res->y++;
    res->x = 0;
    res->MoveCursor(res, res->x, res->y);
    return;
  } else if (str[0] == '\b') {
    if (res->x > 0) {
      res->x--;
      int bmx = res->x;
      int bmy = res->y;
      SDraw_Box(sht->buf, res->x * 8, res->y * 16, res->x * 8 + 8,
                res->y * 16 + 16, color_table[res->color >> 4], res->xsize * 8);
      res->x++;
      put_sht(sht, bmx * 8, bmy * 16, 1);
      bmx = res->x;
      bmy = res->y;
      SDraw_Box(sht->buf, res->x * 8, res->y * 16, res->x * 8 + 8,
                res->y * 16 + 16, color_table[res->color >> 4], res->xsize * 8);
      res->x++;
      put_sht(sht, bmx * 8, bmy * 16, 1);
      res->x -= 2;
    }
    res->MoveCursor(res, res->x, res->y);
    return;
  }
  if (GetCHorEN(str) == 1) {
    int bmx = res->x;
    int bmy = res->y;
    if (res->x == res->xsize - 1) {
      res->y++;
      res->x = 0;
      res->MoveCursor(res, res->x, res->y);
    }
    SDraw_Box(sht->buf, res->x * 8, res->y * 16, res->x * 8 + 16,
              res->y * 16 + 16, color_table[res->color >> 4], res->xsize * 8);
    PUTCHINESE0(sht->buf, res->x * 8, res->y * 16,
                color_table[res->color & 0xf], c, res->xsize * 8);
    res->x += 2;
    put_sht(sht, bmx * 8, bmy * 16, 2);
    res->MoveCursor(res, res->x, res->y);
    return;
  }
  c = p[0];
  int bmx = res->x;
  int bmy = res->y;
  SDraw_Box(sht->buf, res->x * 8, res->y * 16, res->x * 8 + 8, res->y * 16 + 16,
            color_table[res->color >> 4], res->xsize * 8);
  SDraw_Char(sht->buf, res->x * 8, res->y * 16, c,
             color_table[res->color & 0xf], res->xsize * 8);
  res->x++;
  put_sht(sht, bmx * 8, bmy * 16, 1);
  res->MoveCursor(res, res->x, res->y);
}
void MoveCursor_HighTextMode(struct tty *res, int x, int y) {
  res->x = x;
  res->y = y;
  sheet_slide(sht_cur, res->x * 8, res->y * 16);
}
void Draw_Box_HighTextMode(struct tty *res, int x, int y, int x1, int y1,
                           unsigned char color) {
  io_cli();
  struct SHEET *sht = (struct SHEET *)res->vram;
  for (int i = y * 16; i <= y1 * 16; i++) {
    for (int j = x * 8; j <= x1 * 8; j++) {
      if (sht->buf[i * sht->bxsize + j] == color_table[res->color & 0xf]) {
        sht->buf[i * sht->bxsize + j] = color_table[color & 0xf];
      } else {
        sht->buf[i * sht->bxsize + j] = color_table[color >> 4];
      }
    }
  }
  sheet_refresh(sht, x * 8, y * 16, x1 * 8, y1 * 16);
  io_sti();
}
/*void Gar_Test_Task() {
  char fifo_buf[128];
  struct FIFO8 fifo;
  fifo8_init(&fifo, 128, fifo_buf);
  struct TIMER *timer;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 250);
  while (1) {
    sheet_slide(sht_cur, tty_h->x * 8, tty_h->y * 16);
    if (fifo8_status(&fifo) != 0) {
      int i = fifo8_get(&fifo);
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
}*/
void SwitchToHighTextMode() {
  if (set_mode(1024, 768, 32) != 0) {
    printf("Can't enable 1024x768x32 VBE mode.\n\n");
    return;
  }
  struct VBEINFO *vinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
  shtctl0 = shtctl_init(vinfo->vram, vinfo->xsize, vinfo->ysize);
  vram_t *scr_buf =
      page_malloc((vinfo->xsize + 1) * (vinfo->ysize + 1) * sizeof(color_t));
  vram_t *cur_buf = page_malloc(16 * 32 * sizeof(color_t));
  struct SHEET *sht_scr = sheet_alloc(shtctl0);
  sht_cur = sheet_alloc(shtctl0);
  sheet_setbuf(sht_scr, scr_buf, vinfo->xsize, vinfo->ysize, -1);
  sheet_setbuf(sht_cur, cur_buf, 8, 16, 99);
  memset(scr_buf, 0, vinfo->xsize * vinfo->ysize * sizeof(color_t));
  Draw_Cur(cur_buf, 0, 0, 8);
  sheet_slide(sht_scr, 0, 0);
  sheet_slide(sht_cur, 0, 0);
  sheet_updown(sht_scr, 0);
  sheet_updown(sht_cur, 1);
  /*stack = (unsigned int)page_malloc(64 * 1024);
  t1 = AddTask("t1", 1, 2 * 8, (int)Gar_Test_Task, 1 * 8, 1 * 8,
               stack + 64 * 1024);*/
  struct tty *tty_h = tty_alloc((void *)sht_scr, vinfo->xsize / 8,
                                vinfo->ysize / 16, putchar_HighTextMode,
                                MoveCursor_HighTextMode, clear_HighTextMode,
                                screen_ne_HighTextMode, Draw_Box_HighTextMode);
  tty_set_default(tty_h);
  tty_set(current_task(), tty_h);
}
void AddShell_HighTextMode() {
  io_cli();
  struct TASK *task = current_task();
  vram_t *vram = page_kmalloc((task->TTY->xsize * 8 + 1) *
                              (task->TTY->ysize * 16 + 1) * sizeof(color_t));
  struct SHEET *sht_scr = sheet_alloc(shtctl0);
  sheet_setbuf(sht_scr, vram, task->TTY->xsize * 8, task->TTY->ysize * 16, -2);
  sheet_updown(sht_scr, -1);
  struct tty *ntty = tty_alloc((void *)sht_scr, task->TTY->xsize,
                               task->TTY->ysize, putchar_HighTextMode,
                               MoveCursor_HighTextMode, clear_HighTextMode,
                               screen_ne_HighTextMode, Draw_Box_HighTextMode);
  struct TASK *ntask =
      AddTask("Shell", 1, 2 * 8, (int)shell_handler, 1 * 8, 1 * 8,
              (unsigned int)page_kmalloc(128 * 1024) + 128 * 1024);
  char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
  char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
  char *kbuf = (char *)page_kmalloc(4096);
  char *mbuf = (char *)page_kmalloc(4096);
  fifo8_init(kfifo, 4096, kbuf);
  fifo8_init(mfifo, 4096, mbuf);
  TaskSetFIFO(ntask, kfifo, mfifo);
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  ntask->alloc_addr = alloc_addr;
  ntask->alloc_size = 512 * 1024;
  init_mem(ntask);
  ntask->fifosleep = 3;
  int fg = tty_set(ntask, ntty);
  //  printk("set vram = %08x\n",ntty->vram);
  ntty->clear(ntty);
  io_sti();
}
void SwitchShell_HighTextMode(int i) {
  io_cli();
  extern struct List *tty_list;
  extern struct tty *tty_default;
  struct tty *t = (struct tty *)FindForCount(i + 1, tty_list)->val;
  struct tty *n = NULL;
  for (int j = 1; FindForCount(j, tty_list) != 0; j++) {
    n = (struct tty *)FindForCount(j, tty_list)->val;
    struct SHEET *sht = (struct SHEET *)n->vram;
    if (sht->height == 0) {
      break;
    } else {
      n = NULL;
    }
  }
  if (n == NULL) {
    n = tty_default;
  }
  if (n == t) {
    return;
  }
  // 交换
  struct SHEET *sht_t = (struct SHEET *)t->vram;
  struct SHEET *sht_n = (struct SHEET *)n->vram;
  sheet_updown(sht_n, -1);
  sheet_updown(sht_t, 0);
  for (int j = 1; GetTask(j) != 0; j++) {
    struct TASK *task = GetTask(j);
    if (task->TTY == t && (strcmp("Shell", task->name) == 0 ||
                           (task->app == 1 && task->forever == 0))) {
      task->sleep = 0;
      if (task->fifosleep == 3) {
        task->fifosleep = 0;
      }
    } else if ((task->TTY == n || task->TTY->using1 != 1) &&
               (strcmp("Shell", task->name) == 0 ||
                (task->app == 1 && task->forever == 0))) {
      if (task->fifosleep == 0) {
        task->fifosleep = 3;
      }
    }
  }
  t->MoveCursor(t, t->x, t->y);
  io_sti();
}
bool now_tty_HighTextMode(struct tty *res) {
  struct SHEET *sht = (struct SHEET *)res->vram;
  if (sht->height == 0) {
    return true;
  } else {
    return false;
  }
}
